// @(#)root/proof:$Id$
// Author: Jan Fiete Grosse-Oetringhaus, 08.08.07

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofDataSetManager
#define ROOT_TProofDataSetManager

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofDataSetManager                                                 //
//                                                                      //
// This class contains functions to handle datasets in PROOF            //
// It is the layer between TProofServ and the file system that stores   //
// the datasets.                                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif
#ifndef ROOT_TMap
#include "TMap.h"
#endif
#ifndef ROOT_TSystem
#include "TSystem.h"
#endif

class TFileCollection;
class TMD5;
class TMessage;
class TSocket;
class TVirtualMonitoringWriter;

class TProofDataSetManager : public TObject
{
private:
   Bool_t  fSilent;       // set flag to disable output
   TString fDataSetDir;   // location of datasets
   TString fGroup;        // Group to which the owner of this session belongs
   TString fUser;         // Owner of the session
   TString fMSSUrl;       // URL for the Mass Storage System
   TString fCommonUser;   // user that stores the COMMON datasets
   TString fCommonGroup;  // group that stores the COMMON datasets

   Bool_t  fCheckQuota;
   TMap    fGroupQuota;   // group quotas (read from config file)

   TMap    fGroupUsed;    // <group> --> <used bytes> (TParameter)
   TMap    fUserUsed;     // <group> --> <map of users> --> <value>

   Int_t   fNTouchedFiles; // number of files touched in the last ScanDataSet operation
   Int_t   fNOpenedFiles;  // number of files opened in the last ScanDataSet operation
   Int_t   fNDisappearedFiles; // number of files disappared in the last ScanDataSet operation

   TString fGroupConfigFile;  // Path to the group config file
   Long_t  fMTimeGroupConfig; // Last modification of the group config file

   static TString fgDataSetLockFile;   // dataset lock file
   static Int_t   fgLockFileTimeLimit; // limit in seconds after a lock automatically expires
   static TString fgCommonDataSetTag;  // name for common datasets, default: COMMON

   Bool_t  ReadGroupConfig(const char *cf = 0);
   Bool_t  TestPermissions(FileStat_t st, EAccessMode mode = kFileExists);

   TProofDataSetManager(const TProofDataSetManager&);             // not implemented
   TProofDataSetManager& operator=(const TProofDataSetManager&);  // not implemented

protected:
   const char *GetDataSetPath(const char *group, const char *user, const char *dsName);
   void GetQuota(const char *group, const char *user, const char *dsName, TFileCollection *dataset);
   Bool_t BrowseDataSets(const char *group, const char *user, const char *option, TMap *target);

public:
   TProofDataSetManager(const char *dataSetDir,
                        const char *group, const char *user, Bool_t silent = kFALSE);
   virtual ~TProofDataSetManager() { }

   TFileCollection *GetDataSet(const char *uri);
   TFileCollection *GetDataSet(const char *group, const char *user, const char *dsName,
                               const char *option = "", TMD5 **checksum = 0);
   Int_t    WriteDataSet(const char *group, const char *user, const char *dsName,
                         TFileCollection *dataset, const char *option = "", TMD5 *checksum = 0);
   Bool_t   RemoveDataSet(const char *group, const char *user, const char *dsName);
   Bool_t   ExistsDataSet(const char *group, const char *user, const char *dsName);
   Int_t    ScanDataSet(TFileCollection *dataset, const char *option = "");
   Int_t    ScanDataSet(const char *group,
                        const char *user, const char *dsName, const char *option = "");

   TMap    *GetDataSets(const char *group = 0, const char *user = 0, const char *option = "");

   void     SetSilent(Bool_t flag) { fSilent = flag; }

   void     PrintUsedSpace();
   void     MonitorUsedSpace(TVirtualMonitoringWriter *monitoring);
   void     UpdateUsedSpace();

   TMap    *GetGroupUsedMap() { return &fGroupUsed; }
   TMap    *GetUserUsedMap() { return &fUserUsed; }

   Long64_t GetGroupUsed(const char *group);
   Long64_t GetGroupQuota(const char *group);

   TMap    *GetGroupQuotaMap() { return &fGroupQuota; }

   Int_t    GetNTouchedFiles() { return fNTouchedFiles; }
   Int_t    GetNOpenedFiles() { return fNOpenedFiles; }
   Int_t    GetNDisapparedFiles() { return fNDisappearedFiles; }

   Bool_t   ParseDataSetUri(const char *uri, TString *dsGroup = 0, TString *dsUser = 0,
                            TString *dsName = 0, TString *dsTree = 0,
                            Bool_t onlyCurrent = kFALSE, Bool_t wildcards = kFALSE);

   Int_t    HandleRequest(TMessage *msg, TSocket *sock, FILE *flog);

   ClassDef(TProofDataSetManager, 0)
};

#endif
