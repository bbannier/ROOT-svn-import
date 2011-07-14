// @(#)root/proofplayer:$Id$
// Author: Kristjan Gulbrandsen   11/05/04

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TPerfStats                                                           //
//                                                                      //
// Provides the interface for the PROOF internal performance measurment //
// and event tracing.                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TPerfStats.h"

#include "Riostream.h"
#include "TCollection.h"
#include "TEnv.h"
#include "TError.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TDSet.h"
#include "TProofDebug.h"
#include "TProof.h"
#include "TProofServ.h"
#include "TSlave.h"
#include "TStatus.h"
#include "TTree.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TParameter.h"
#include "TPluginManager.h"
#include "TROOT.h"
#include "TTimeStamp.h"
#include "TVirtualMonitoring.h"


ClassImp(TPerfEvent)
ClassImp(TPerfStats)


//------------------------------------------------------------------------------

//______________________________________________________________________________
TPerfEvent::TPerfEvent(TTimeStamp *offset)
   : fEvtNode("-3"), fType(TVirtualPerfStats::kUnDefined), fSlave(),
     fEventsProcessed(0), fBytesRead(0), fLen(0), fLatency(0.0), fProcTime(0.0), fCpuTime(0.0),
     fIsStart(kFALSE), fIsOk(kFALSE)
{
   // Constructor

   if (gProofServ != 0) {
      fEvtNode = gProofServ->GetOrdinal();
   } else {
      if (gProof && gProof->IsLite())
         fEvtNode = "0";
      else
         fEvtNode = "-2"; // not on a PROOF server
   }

   if (offset != 0) {
      fTimeStamp = TTimeStamp(fTimeStamp.GetSec() - offset->GetSec(),
                     fTimeStamp.GetNanoSec() - offset->GetNanoSec());
   }
}

//______________________________________________________________________________
Int_t TPerfEvent::Compare(const TObject *obj) const
{
   // Compare method. Must return -1 if this is smaller than obj,
   // 0 if objects are equal and 1 if this is larger than obj.

   const TPerfEvent *pe = dynamic_cast<const TPerfEvent*>(obj);

   if (!pe) {
      Error("Compare", "input is not a TPerfEvent object");
      return 0;
   }

   if (fTimeStamp < pe->fTimeStamp) {
      return -1;
   } else if (fTimeStamp == pe->fTimeStamp) {
      return 0;
   } else {
      return 1;
   }
}

//______________________________________________________________________________
void TPerfEvent::Print(Option_t *) const
{
   // Dump content of this instance
        
   TString where;
   if (fEvtNode == -2) {
      where = "TPerfEvent: StandAlone ";
   } else if ( fEvtNode == -1 ) {
      where = "TPerfEvent: Master ";
   } else {
      where.Form("TPerfEvent: Worker %s ", fEvtNode.Data());
   }
   Printf("%s %s %f", where.Data(),
                      TVirtualPerfStats::EventType(fType), double(fTimeStamp));
}

Long_t TPerfStats::fgVirtMemMax = -1;
Long_t TPerfStats::fgResMemMax = -1;

//______________________________________________________________________________
TPerfStats::TPerfStats(TList *input, TList *output)
   : fTrace(0), fPerfEvent(0), fPacketsHist(0), fEventsHist(0), fLatencyHist(0),
      fProcTimeHist(0), fCpuTimeHist(0), fBytesRead(0),
      fTotCpuTime(0.), fTotBytesRead(0), fTotEvents(0), fNumEvents(0),
      fSlaves(0), fDoHist(kFALSE),
      fDoTrace(kFALSE), fDoTraceRate(kFALSE), fDoSlaveTrace(kFALSE), fDoQuota(kFALSE),
      fMonitorPerPacket(kFALSE), fMonitoringWriters(3),
      fDataSet("+++none+++"), fDataSetSize(-1), fOutput(output)
{
   // Normal constructor.

   TProof *proof = (gProofServ) ? gProofServ->GetProof() : gProof;

   // Master flag
   Bool_t isMaster = ((proof && proof->TestBit(TProof::kIsMaster)) ||
                      (gProofServ && gProofServ->IsMaster())) ? kTRUE : kFALSE;

   TList *l = proof ? proof->GetListOfSlaveInfos() : 0 ;
   TIter nextslaveinfo(l);
   while (TSlaveInfo *si = dynamic_cast<TSlaveInfo*>(nextslaveinfo()))
      if (si->fStatus == TSlaveInfo::kActive) fSlaves++;

   PDB(kGlobal,1) Info("TPerfStats", "Statistics for %d slave(s)", fSlaves);

   fDoHist = (input->FindObject("PROOF_StatsHist") != 0);
   fDoTrace = (input->FindObject("PROOF_StatsTrace") != 0);
   fDoTraceRate = (input->FindObject("PROOF_RateTrace") != 0);
   fDoSlaveTrace = (input->FindObject("PROOF_SlaveStatsTrace") != 0);

   // Check per packet monitoring
   Int_t perpacket = -1;
   if (TProof::GetParameter(input, "PROOF_MonitorPerPacket", perpacket) != 0) {
      // Check if there is a global monitor-per-packet setting
      perpacket = gEnv->GetValue("Proof.MonitorPerPacket", 0);
   }
   fMonitorPerPacket = (perpacket == 1) ? kTRUE : kFALSE;
   if (fMonitorPerPacket)
      Info("TPerfStats", "sending full information after each packet");
 
   // Check what information to send
   fMonitorInfo = gEnv->GetValue("Proof.MonitorInfo", 1);
   if (fMonitorInfo == 0)
      Info("TPerfStats", "sending old-structured information (no {memory,dataset,status} info)");
   
   // Extract the name of the dataset
   if (fMonitorInfo > 0) {
      TObject *o = 0;
      TIter nxi(input);
      while ((o = nxi()))
         if (!strncmp(o->ClassName(), "TDSet", strlen("TDSet"))) break;
      if (o) {
         TDSet *dset = (TDSet *) o;
         fDataSetSize = dset->GetNumOfFiles();
         if (fDataSetSize > 0) {
            fDataSet = "";
            TString grus = (gProofServ) ? TString::Format("/%s/%s/", gProofServ->GetGroup(),
                                                                     gProofServ->GetUser()) : "";
            TString dss = dset->GetName(), ds;
            Ssiz_t fd = 0, nq = kNPOS;
            while (dss.Tokenize(ds, fd, "[,| ]")) {
               if ((nq = ds.Index("?")) != kNPOS) ds.Remove(nq);
               ds.ReplaceAll(grus, "");
               if (!fDataSet.IsNull()) fDataSet += ",";
               fDataSet += ds;
            }
         }
      }

      // Dataset string limited in length: get the authorized size
      fDataSetLen = gEnv->GetValue("Proof.Monitor.DataSetLen", 512);
      if (fDataSetLen != 512)
         Info("TPerfStats", "dataset string length truncated to %d chars", fDataSetLen);
      if (fDataSet.Length() > fDataSetLen) fDataSet.Resize(fDataSetLen);
      //
      PDB(kGlobal,1)
         Info("TPerfStats", "dataset: '%s', # files: %d", fDataSet.Data(), fDataSetSize);
   }
 
   if ((isMaster && (fDoTrace || fDoTraceRate)) || (!isMaster && fDoSlaveTrace)) {
      // Construct tree
      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_PerfStats"));
      fTrace = new TTree("PROOF_PerfStats", "PROOF Statistics");
      fTrace->SetDirectory(0);
      fTrace->Bronch("PerfEvents", "TPerfEvent", &fPerfEvent, 64000, 0);
      output->Add(fTrace);
   }

   if (fDoHist && isMaster) {
      // Make Histograms
      Double_t time_per_bin = 1e-3; // 10ms
      Double_t min_time = 0;
      Int_t ntime_bins = 1000;

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_PacketsHist"));
      fPacketsHist = new TH1D("PROOF_PacketsHist", "Packets processed per Worker",
                              fSlaves, 0, fSlaves);
      fPacketsHist->SetDirectory(0);
      fPacketsHist->SetMinimum(0);
      output->Add(fPacketsHist);

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_EventsHist"));
      fEventsHist = new TH1D("PROOF_EventsHist", "Events processed per Worker",
                             fSlaves, 0, fSlaves);
      fEventsHist->SetFillColor(kGreen);
      fEventsHist->SetDirectory(0);
      fEventsHist->SetMinimum(0);
      output->Add(fEventsHist);

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_NodeHist"));
      fNodeHist = new TH1D("PROOF_NodeHist", "Slaves per Fileserving Node",
                           fSlaves, 0, fSlaves);
      fNodeHist->SetDirectory(0);
      fNodeHist->SetMinimum(0);
      fNodeHist->SetBit(TH1::kCanRebin);
      output->Add(fNodeHist);

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_LatencyHist"));
      fLatencyHist = new TH2D("PROOF_LatencyHist", "GetPacket Latency per Worker",
                              fSlaves, 0, fSlaves,
                              ntime_bins, min_time, time_per_bin);
      fLatencyHist->SetDirectory(0);
      fLatencyHist->SetMarkerStyle(4);
      fLatencyHist->SetBit(TH1::kCanRebin);
      output->Add(fLatencyHist);

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_ProcTimeHist"));
      fProcTimeHist = new TH2D("PROOF_ProcTimeHist", "Packet Processing Time per Worker",
                               fSlaves, 0, fSlaves,
                               ntime_bins, min_time, time_per_bin);
      fProcTimeHist->SetDirectory(0);
      fProcTimeHist->SetMarkerStyle(4);
      fProcTimeHist->SetBit(TH1::kCanRebin);
      output->Add(fProcTimeHist);

      gDirectory->RecursiveRemove(gDirectory->FindObject("PROOF_CpuTimeHist"));
      fCpuTimeHist = new TH2D("PROOF_CpuTimeHist", "Packet CPU Time per Worker",
                              fSlaves, 0, fSlaves,
                              ntime_bins, min_time, time_per_bin);
      fCpuTimeHist->SetDirectory(0);
      fCpuTimeHist->SetMarkerStyle(4);
      fCpuTimeHist->SetBit(TH1::kCanRebin);
      output->Add(fCpuTimeHist);

      nextslaveinfo.Reset();
      Int_t slavebin=1;
      while (TSlaveInfo *si = dynamic_cast<TSlaveInfo*>(nextslaveinfo())) {
         if (si->fStatus == TSlaveInfo::kActive) {
            fPacketsHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            fEventsHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            fNodeHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            fLatencyHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            fProcTimeHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            fCpuTimeHist->GetXaxis()->SetBinLabel(slavebin, si->GetOrdinal());
            slavebin++;
         }
      }
   }

   if (isMaster) {

      // Monitoring for query performances using monitoring system (e.g. Monalisa, SQL, ...)
      //
      // We support multiple specifications separated by ',' or '|' or '\' (the latter need
      // top be escaped three times in the regular experession), e.g.
      // ProofServ.Monitoring:  Monalisa bla bla bla,
      // +ProofServ.Monitoring:  SQL blu blu blu
      
      TString mons = gEnv->GetValue("ProofServ.Monitoring", ""), mon;
      Ssiz_t fmon = 0;
      TVirtualMonitoringWriter *monWriter = 0;
      while (mons.Tokenize(mon, fmon, "[,|\\\\]")) {
         if (mon != "") {
            // Extract arguments (up to 9 'const char *')
            TString a[10];
            Int_t from = 0;
            TString tok;
            Int_t na = 0;
            while (mon.Tokenize(tok, from, " "))
               a[na++] = tok;
            na--;
            // Get monitor object from the plugin manager
            TPluginHandler *h = 0;
            if ((h = gROOT->GetPluginManager()->FindHandler("TVirtualMonitoringWriter", a[0]))) {
               if (h->LoadPlugin() != -1) {
                  monWriter =
                     (TVirtualMonitoringWriter *) h->ExecPlugin(na, a[1].Data(), a[2].Data(), a[3].Data(),
                                                                  a[4].Data(), a[5].Data(), a[6].Data(),
                                                                  a[7].Data(), a[8].Data(), a[9].Data());
                  if (monWriter && monWriter->IsZombie()) SafeDelete(monWriter);
               }
            }
         }

         if (monWriter) {
            fMonitoringWriters.Add(monWriter);
            PDB(kGlobal,1)
               Info("TPerfStats", "created monitoring object: %s - # of active monitors: %d",
                                  mon.Data(), fMonitoringWriters.GetEntries());
            fDoQuota = kTRUE;
         }
         monWriter = 0;
      }
   }
}

//______________________________________________________________________________
void TPerfStats::SimpleEvent(EEventType type)
{
   // Simple event.

   if (type == kStop && fPacketsHist != 0) {
      fNodeHist->LabelsDeflate("X");
      fNodeHist->LabelsOption("auv","X");
   }

   if (type == kStop && fDoQuota)
      WriteQueryLog();

   if (fTrace == 0) return;

   TPerfEvent pe(&fTzero);
   pe.fType = type;

   fPerfEvent = &pe;
   fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
   fTrace->Fill();
   fPerfEvent = 0;
}

//______________________________________________________________________________
void TPerfStats::PacketEvent(const char *slave, const char* slavename, const char* filename,
                             Long64_t eventsprocessed, Double_t latency, Double_t proctime,
                             Double_t cputime, Long64_t bytesRead)
{
   // Packet event.
   // See WriteQueryLog for the descripition of the structure sent for monitoring
   // when fMonitorPerPacket is kTRUE.

   if (fDoTrace && fTrace != 0) {
      TPerfEvent pe(&fTzero);

      pe.fType = kPacket;
      pe.fSlaveName = slavename;
      pe.fFileName = filename;
      pe.fSlave = slave;
      pe.fEventsProcessed = eventsprocessed;
      pe.fBytesRead = bytesRead;
      pe.fLatency = latency;
      pe.fProcTime = proctime;
      pe.fCpuTime = cputime;

      fPerfEvent = &pe;
      fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
      fTrace->Fill();
      fPerfEvent = 0;
   }

   PDB(kGlobal,1)
      Info("PacketEvent","%s: fDoHist: %d, fPacketsHist: %p, eventsprocessed: %lld",
                         slave, fDoHist, fPacketsHist, eventsprocessed);

   if (fDoHist && fPacketsHist != 0) {
      fPacketsHist->Fill(slave, 1);
      fEventsHist->Fill(slave, eventsprocessed);
      fLatencyHist->Fill(slave, latency, 1);
      fProcTimeHist->Fill(slave, proctime, 1);
      fCpuTimeHist->Fill(slave, cputime, 1);
   }

   if (fDoQuota) {
      fTotCpuTime += cputime;
      fTotBytesRead += bytesRead;
      fTotEvents += eventsprocessed;
   }

   // Write to monitoring system, if requested
   if (!fMonitoringWriters.IsEmpty() && fMonitorPerPacket) {
      TQueryResult *qr = (gProofServ && gProofServ->GetProof()) ?
                          gProofServ->GetProof()->GetQueryResult() : 0;
      if (!gProofServ || !gProofServ->GetSessionTag() || !gProofServ->GetProof() || !qr) {
         Error("PacketEvent", "some required object are undefined (%p %p %p %p)",
               gProofServ, (gProofServ ? gProofServ->GetSessionTag() : 0),
              (gProofServ ? gProofServ->GetProof() : 0),
              ((gProofServ && gProofServ->GetProof()) ? qr : 0));
         return;
      }
      
      TTimeStamp stop;
      TString identifier;
      identifier.Form("%s-q%d", gProofServ->GetSessionTag(), qr->GetSeqNum());

      TList values;
      values.SetOwner();
      values.Add(new TParameter<int>("id", 0));
      values.Add(new TNamed("user", gProofServ->GetUser()));
      values.Add(new TNamed("proofgroup", gProofServ->GetGroup()));
      values.Add(new TNamed("begin", fTzero.AsString("s")));
      values.Add(new TNamed("end", stop.AsString("s")));
      values.Add(new TParameter<int>("walltime", stop.GetSec()-fTzero.GetSec()));
      values.Add(new TParameter<Long64_t>("bytesread", fTotBytesRead));
      values.Add(new TParameter<Long64_t>("events", fTotEvents));
      values.Add(new TParameter<Long64_t>("totevents", fNumEvents));
      values.Add(new TParameter<int>("workers", fSlaves));
      values.Add(new TNamed("querytag", identifier.Data()));
      
      // If the new style has been requested
      if (fMonitorInfo > 0) {
         // Memory usage on workers
         TStatus *pst = (fOutput) ? (TStatus *) fOutput->FindObject("PROOF_Status") : 0;
         // This most likely will be always NULL when sending from GetNextPacket ...
         Long64_t vmxw = (pst) ? (Long64_t) pst->GetVirtMemMax() : -1;
         Long64_t rmxw = (pst) ? (Long64_t) pst->GetResMemMax() : -1;
         values.Add(new TParameter<Long64_t>("vmemmxw", vmxw));
         values.Add(new TParameter<Long64_t>("rmemmxw", rmxw));
         // Memory usage on master
         values.Add(new TParameter<Long64_t>("vmemmxm", (Long64_t) fgVirtMemMax));
         values.Add(new TParameter<Long64_t>("rmemmxm", (Long64_t) fgResMemMax));
         // Dataset information
         values.Add(new TNamed("dataset", fDataSet.Data()));
         values.Add(new TParameter<int>("numfiles", fDataSetSize));
         // Query status
         Int_t est = (pst) ? pst->GetExitStatus() : -1;
         values.Add(new TParameter<int>("status", est));
      }

      for (Int_t i = 0; i < fMonitoringWriters.GetEntries(); i++) {
         TVirtualMonitoringWriter *m = (TVirtualMonitoringWriter *) fMonitoringWriters[i];
         if (m && !m->SendParameters(&values, identifier)) {
            Error("PacketEvent", "sending of monitoring info failed (%s)", m->GetName());
         } else if (!m) {
            Warning("PacketEvent", "undefined entry found in monitors array for id: %d", i);
         }
      }
   }
}

//______________________________________________________________________________
void TPerfStats::FileEvent(const char *slave, const char *slavename, const char *nodename,
                            const char *filename, Bool_t isStart)
{
   // File event.

   if (fDoTrace && fTrace != 0) {
      TPerfEvent pe(&fTzero);

      pe.fType = kFile;
      pe.fSlaveName = slavename;
      pe.fNodeName = nodename;
      pe.fFileName = filename;
      pe.fSlave = slave;
      pe.fIsStart = isStart;

      fPerfEvent = &pe;
      fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
      fTrace->Fill();
      fPerfEvent = 0;
   }

   if (fDoHist && fPacketsHist != 0) {
      fNodeHist->Fill(nodename, isStart ? 1 : -1);
   }
}

//______________________________________________________________________________
void TPerfStats::FileOpenEvent(TFile *file, const char *filename, Double_t start)
{
   // Open file event.

   if (fDoTrace && fTrace != 0) {
      TPerfEvent pe(&fTzero);

      pe.fType = kFileOpen;
      pe.fFileName = filename;
      pe.fFileClass = file != 0 ? file->ClassName() : "none";
      pe.fProcTime = double(TTimeStamp())-start;
      pe.fIsOk = (file != 0);

      fPerfEvent = &pe;
      fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
      fTrace->Fill();
      fPerfEvent = 0;
   }
}

//______________________________________________________________________________
void TPerfStats::FileReadEvent(TFile *file, Int_t len, Double_t start)
{
   // Read file event.

   if (fDoTrace && fTrace != 0) {
      TPerfEvent pe(&fTzero);

      pe.fType = kFileRead;
      pe.fFileName = file->GetName();
      pe.fFileClass = file->ClassName();
      pe.fLen = len;
      pe.fProcTime = double(TTimeStamp())-start;

      fPerfEvent = &pe;
      fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
      fTrace->Fill();
      fPerfEvent = 0;
   }
}

//______________________________________________________________________________
void TPerfStats::FileUnzipEvent(TFile * /* file */, Long64_t /* pos */, Double_t /* start */, Int_t /* complen */, Int_t /* objlen */)
{
   // Record TTree file unzip event.
   // start is the TimeStamp before unzip
   // pos is where in the file the compressed buffer came from
   // complen is the length of the compressed buffer
   // objlen is the length of the de-compressed buffer

   // Do nothing for now.
}

//______________________________________________________________________________
void TPerfStats::RateEvent(Double_t proctime, Double_t deltatime,
                           Long64_t eventsprocessed, Long64_t bytesRead)
{
   // Rate event.

   if ((fDoTrace || fDoTraceRate) && fTrace != 0) {
      TPerfEvent pe(&fTzero);

      pe.fType = kRate;
      pe.fEventsProcessed = eventsprocessed;
      pe.fBytesRead = bytesRead;
      pe.fProcTime = proctime;
      pe.fLatency = deltatime;

      fPerfEvent = &pe;
      fTrace->SetBranchAddress("PerfEvents",&fPerfEvent);
      fTrace->Fill();
      fPerfEvent = 0;
   }
}

//______________________________________________________________________________
void TPerfStats::SetBytesRead(Long64_t num)
{
   // Set number of bytes read.

   fBytesRead = num;
}

//______________________________________________________________________________
Long64_t TPerfStats::GetBytesRead() const
{
   // Get number of bytes read.

   return fBytesRead;
}

//______________________________________________________________________________
void TPerfStats::WriteQueryLog()
{
   // Connect to SQL server and register query log used for quotas.
   // The <proofquerylog> table has the format:
   // CREATE TABLE <proofquerylog> (
   //   id            INT NOT NULL PRIMARY KEY AUTO_INCREMENT,
   //   user          VARCHAR(32) NOT NULL,
   //   proofgroup    VARCHAR(32),
   //   begin         DATETIME,
   //   end           DATETIME,
   //   walltime      INT,
   //   cputime       FLOAT,
   //   bytesread     BIGINT,
   //   events        BIGINT,
   //   workers       INT
   //   querytag      VARCHAR(64) NOT NULL,
   //   vmemmxw       BIGINT,                  (*)
   //   rmemmxw       BIGINT,                  (*)
   //   vmemmxm       BIGINT,                  (*)
   //   rmemmxm       BIGINT,                  (*)
   //   dataset       VARCHAR(512),            (*, **)
   //   numfiles      INT                      (*)
   //   status        INT                      (*)
   // )
   // (*) Only for fMonitorInfo > 0
   // (**) Size controlled by variable Proof.Monitor.DataSetLen .
   // The name of the table is set while initializing the monitor writer object.
   // The same info is send to Monalisa (or other monitoring systems) in the
   // form of a list of name,value pairs.

   TTimeStamp stop;

   // Write to monitoring system
   if (!fMonitoringWriters.IsEmpty()) {
      TQueryResult *qr = (gProofServ && gProofServ->GetProof()) ?
                          gProofServ->GetProof()->GetQueryResult() : 0;
      if (!gProofServ || !gProofServ->GetSessionTag() || !gProofServ->GetProof() || !qr) {
         Error("WriteQueryLog", "some required object are undefined (%p %p %p %p)",
               gProofServ, (gProofServ ? gProofServ->GetSessionTag() : 0),
              (gProofServ ? gProofServ->GetProof() : 0),
              ((gProofServ && gProofServ->GetProof()) ? qr : 0));
         return;
      }

      TString identifier;
      identifier.Form("%s-q%d", gProofServ->GetSessionTag(), qr->GetSeqNum());

      TList values;
      values.SetOwner();
      values.Add(new TParameter<int>("id", 0));
      values.Add(new TNamed("user", gProofServ->GetUser()));
      values.Add(new TNamed("proofgroup", gProofServ->GetGroup()));
      values.Add(new TNamed("begin", fTzero.AsString("s")));
      values.Add(new TNamed("end", stop.AsString("s")));
      values.Add(new TParameter<int>("walltime", stop.GetSec()-fTzero.GetSec()));
      values.Add(new TParameter<float>("cputime", fTotCpuTime));
      values.Add(new TParameter<Long64_t>("bytesread", fTotBytesRead));
      values.Add(new TParameter<Long64_t>("events", fTotEvents));
      values.Add(new TParameter<Long64_t>("totevents", fTotEvents));
      values.Add(new TParameter<int>("workers", fSlaves));
      values.Add(new TNamed("querytag", identifier.Data()));
      
      // If the new style has been requested
      if (fMonitorInfo > 0) {
         // Memory usage on workers
         TStatus *pst = (fOutput) ? (TStatus *) fOutput->FindObject("PROOF_Status") : 0;
         Long64_t vmxw = (pst) ? (Long64_t) pst->GetVirtMemMax() : -1;
         Long64_t rmxw = (pst) ? (Long64_t) pst->GetResMemMax() : -1;
         values.Add(new TParameter<Long64_t>("vmemmxw", vmxw));
         values.Add(new TParameter<Long64_t>("rmemmxw", rmxw));
         // Memory usage on master
         values.Add(new TParameter<Long64_t>("vmemmxm", (Long64_t) fgVirtMemMax));
         values.Add(new TParameter<Long64_t>("rmemmxm", (Long64_t) fgResMemMax));
         // Dataset information
         values.Add(new TNamed("dataset", fDataSet.Data()));
         values.Add(new TParameter<int>("numfiles", fDataSetSize));
         // Query status
         Int_t est = (pst) ? pst->GetExitStatus() : -1;
         values.Add(new TParameter<int>("status", est));
      }

      for (Int_t i = 0; i < fMonitoringWriters.GetEntries(); i++) {
         TVirtualMonitoringWriter *m = (TVirtualMonitoringWriter *) fMonitoringWriters[i];
         if (m && !m->SendParameters(&values, identifier)) {
            Error("WriteQueryLog", "sending of monitoring info failed (%s)", m->GetName());
         } else if (!m) {
            Warning("WriteQueryLog", "undefined entry found in monitors array for id: %d", i);
         }
      }
   }
}

//______________________________________________________________________________
void TPerfStats::Setup(TList *input)
{
   // Setup the PROOF input list with requested statistics and tracing options.

   const Int_t ntags=3;
   const Char_t *tags[ntags] = {"StatsHist",
                                "StatsTrace",
                                "SlaveStatsTrace"};

   for (Int_t i=0; i<ntags; i++) {
      TString envvar = "Proof.";
      envvar += tags[i];
      TString inputname = "PROOF_";
      inputname += tags[i];
      TObject* obj = input->FindObject(inputname.Data());
      if (gEnv->GetValue(envvar.Data(), 0)) {
         if (!obj)
            input->Add(new TNamed(inputname.Data(),""));
      } else {
         if (obj) {
            input->Remove(obj);
            delete obj;
         }
      }
   }
}

//______________________________________________________________________________
void TPerfStats::Start(TList *input, TList *output)
{
   // Initialize PROOF statistics run.

   if (gPerfStats)
      delete gPerfStats;
   fgVirtMemMax = -1;
   fgResMemMax = -1;
   TPerfStats::SetMemValues();
   
   gPerfStats = new TPerfStats(input, output);
   if (gPerfStats && !gPerfStats->TestBit(TObject::kInvalidObject)) {
      gPerfStats->SimpleEvent(TVirtualPerfStats::kStart);
   } else {
      SafeDelete(gPerfStats);
   }
}

//______________________________________________________________________________
void TPerfStats::Stop()
{
   // Terminate the PROOF statistics run.

   if (!gPerfStats) return;

   TPerfStats::SetMemValues();
   gPerfStats->SimpleEvent(TVirtualPerfStats::kStop);

   delete gPerfStats;
   gPerfStats = 0;
}

//______________________________________________________________________________
void TPerfStats::SetMemValues()
{
   // Record memory usage

   ProcInfo_t pi;
   if (!gSystem->GetProcInfo(&pi)){
      if (pi.fMemVirtual > fgVirtMemMax) fgVirtMemMax = pi.fMemVirtual;
      if (pi.fMemResident > fgResMemMax) fgResMemMax = pi.fMemResident;
   }
}

//______________________________________________________________________________
void TPerfStats::GetMemValues(Long_t &vmax, Long_t &rmax)
{
   // Get memory usage

   vmax = fgVirtMemMax;
   rmax = fgResMemMax;
}
