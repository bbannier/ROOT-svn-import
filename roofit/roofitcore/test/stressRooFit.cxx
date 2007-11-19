// @(#)root/roofitcore:$name:  $:$id$
// Authors: Wouter Verkerke  November 2007

#include "TROOT.h"
#include "TSystem.h"
#include "TStopwatch.h"
#include "TBenchmark.h"
#include "RooPlot.h"
#include "TString.h"
#include "TClass.h"
#include "RooMsgService.h"
#include <iostream>
#include <list>
#include "TFile.h"
#include "RooHist.h"
#include "RooCurve.h"
#include "TWebFile.h"
#include "RooRandom.h"
#include "RooFitResult.h"
using namespace std ;
   
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*//
//                                                                           //
// RooFit Examples, Wouter Verkerke                                          //
//                                                                           //
//                                                                           //
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*_*//


Int_t stressRooFit(const char* refFile, Bool_t writeRef, Int_t doVerbose) ;

static TDirectory* gMemDir = 0 ;

//------------------------------------------------------------------------
void StatusPrint(Int_t id,const TString &title,Bool_t status)
{
  // Print test program number and its title
  const Int_t kMAX = 65;
  Char_t number[4];
  sprintf(number,"%2d",id);
  TString header = TString("Test ")+number+" : "+title;
  const Int_t nch = header.Length();
  for (Int_t i = nch; i < kMAX; i++) header += '.';
  cout << header << (status ? "OK" : "FAILED") << endl;
}



class RooFitTestUnit : public TObject {
public:
  RooFitTestUnit(TFile* refFile, Bool_t writeRef, Int_t verbose) ;
  ~RooFitTestUnit() ;

  void setSilentMode() ;
  void clearSilentMode() ;
  void regPlot(RooPlot* frame, const char* refName) ;  
  void regResult(RooFitResult* r, const char* refName) ;
  Bool_t runTest() ;
  Bool_t runCompTests() ;

  virtual Bool_t testCode() = 0 ;  
protected:
  TFile* _refFile ;
  Bool_t _write ;
  Int_t _verb ;
  list<pair<RooPlot*, string> > _regPlots ;
  list<pair<RooFitResult*, string> > _regResults ;
} ;


RooFitTestUnit::RooFitTestUnit(TFile* refFile, Bool_t writeRef, Int_t verbose) : _refFile(refFile), _write(writeRef), _verb(verbose)
{
}


RooFitTestUnit::~RooFitTestUnit() 
{
}

void RooFitTestUnit::regPlot(RooPlot* frame, const char* refName) 
{
  _regPlots.push_back(make_pair(frame,refName)) ;
}

void RooFitTestUnit::regResult(RooFitResult* r, const char* refName) 
{
  _regResults.push_back(make_pair(r,refName)) ;
}

Bool_t RooFitTestUnit::runCompTests() 
{
  Bool_t ret = kTRUE ;

  list<pair<RooPlot*, string> >::iterator iter = _regPlots.begin() ;
  while (iter!=_regPlots.end()) {

    if (!_write) {

      // Comparison mode
 
     // Retrieve benchmark
      RooPlot* bmark = dynamic_cast<RooPlot*>(_refFile->Get(iter->second.c_str())) ;
      if (!bmark) {
	cout << "stressRooFit ERROR: cannot retrieve RooPlot " << iter->second << " from reference file, skipping " << endl ;
	ret = kFALSE ;
	continue ;
      }
      
      if (_verb) {
	cout << "comparing RooPlot " << iter->first << " to benchmark " << iter->second << " = " << bmark << endl ;      
      }

      Stat_t nItems = iter->first->numItems() ;
      for (Stat_t i=0 ; i<nItems ; i++) {
	TObject* obj = iter->first->getObject((Int_t)i) ;

	// Retrieve corresponding object from reference frame
	TObject* objRef = bmark->findObject(obj->GetName()) ;

	if (!objRef) {
	  cout << "stressRooFit ERROR: cannot retrieve object " << obj->GetName() << " from reference  RooPlot " << iter->second << ", skipping" << endl ;
	  ret = kFALSE ;
	  ++iter ;
	  continue ;
	}

	// Histogram comparisons
	if (obj->IsA()==RooHist::Class()) {
	  RooHist* testHist = static_cast<RooHist*>(obj) ;
	  RooHist* refHist = static_cast<RooHist*>(objRef) ;
	  if (!testHist->isIdentical(*refHist)) {
	    cout << "stressRooFit ERROR: comparison of object " << obj->IsA()->GetName() << "::" << obj->GetName() 
		 <<   " fails comparison with counterpart in reference RooPlot " << bmark->GetName() << endl ;
	    ret = kFALSE ;
	  }
	} else if (obj->IsA()==RooCurve::Class()) {
	  RooCurve* testCurve = static_cast<RooCurve*>(obj) ;
	  RooCurve* refCurve = static_cast<RooCurve*>(objRef) ;
	  if (!testCurve->isIdentical(*refCurve)) {
	    cout << "stressRooFit ERROR: comparison of object " << obj->IsA()->GetName() << "::" << obj->GetName() 
		 <<   " fails comparison with counterpart in reference RooPlot " << bmark->GetName() << endl ;
	    ret = kFALSE ;
	  }
	}	
	
      }

    } else {

      // Writing mode

      cout <<"stressRooFit: Writing reference RooPlot " << iter->first << " as benchmark " << iter->second << endl ;
      _refFile->cd() ;
      iter->first->Write(iter->second.c_str()) ;
      gMemDir->cd() ;
    }
      
    ++iter ;
  }


  list<pair<RooFitResult*, string> >::iterator iter2 = _regResults.begin() ;
  while (iter2!=_regResults.end()) {

    if (!_write) {

      // Comparison mode
 
     // Retrieve benchmark
      RooFitResult* bmark = dynamic_cast<RooFitResult*>(_refFile->Get(iter2->second.c_str())) ;
      if (!bmark) {
	cout << "stressRooFit ERROR: cannot retrieve RooFitResult " << iter2->second << " from reference file, skipping " << endl ;
	++iter2 ;
	ret = kFALSE ;
	continue ;
      }

      if (_verb) {
	cout << "comparing RooFitResult " << iter2->first << " to benchmark " << iter2->second << " = " << bmark << endl ;      
      }

      if (!iter2->first->isIdentical(*bmark)) {
	cout << "stressRooFit ERROR: comparison of object " << iter2->first->IsA()->GetName() << "::" << iter2->first->GetName() 
	     <<   " fails comparison with counterpart in reference RooFitResult " << bmark->GetName() << endl ;
	ret = kFALSE ;
      }
      
        
    } else {

      // Writing mode
      
      cout <<"stressRooFit: Writing reference RooFitResult " << iter2->first << " as benchmark " << iter2->second << endl ;
      _refFile->cd() ;
      iter2->first->Write(iter2->second.c_str()) ;
      gMemDir->cd() ;
    }
      
    ++iter2 ;
  }


  return ret ;
}

void RooFitTestUnit::setSilentMode() 
{
  RooMsgService::instance().setSilentMode(kTRUE) ;
  for (Int_t i=0 ; i<RooMsgService::instance().numStreams() ; i++) {
    RooMsgService::instance().setStreamStatus(i,kFALSE) ;
  }
}

void RooFitTestUnit::clearSilentMode() 
{
  RooMsgService::instance().setSilentMode(kFALSE) ;
  for (Int_t i=0 ; i<RooMsgService::instance().numStreams() ; i++) {
    RooMsgService::instance().setStreamStatus(i,kTRUE) ;
  }  
}


Bool_t RooFitTestUnit::runTest()
{
  gMemDir->cd() ;

  if (_verb<2) { 
    setSilentMode() ;
  } else {
    cout << "*** Begin of output of Unit Test at normal verbosity *************" << endl ;
  }  

  if (!testCode()) return kFALSE ;

  if (_verb<2) { 
    clearSilentMode() ;
  } else {
    cout << "*** End of output of Unit Test at normal verbosity ***************" << endl ;
  }

  return runCompTests() ;
}



#include "TestBasic1.cxx"



//______________________________________________________________________________
Int_t stressRooFit(const char* refFile, Bool_t writeRef, Int_t doVerbose)
{
  // Save memory directory location
  gMemDir = gDirectory ;

  TFile* fref = 0 ;
  if (TString(refFile).Contains("http:")) {
    if (writeRef) {
      cout << "stressRooFit ERROR: reference file must be local file in writing mode" << endl ;
      return kFALSE ;
    }
    fref = new TWebFile(refFile) ;
  } else {
    fref = new TFile(refFile,writeRef?"RECREATE":"") ;
  }
  if (fref->IsZombie()) {
    cout << "stressRooFit ERROR: cannot open reference file " << refFile << endl ;
    return kFALSE ;
  }


  cout << "******************************************************************" <<endl;
  cout << "*  RooFit - S T R E S S suite                                    *" <<endl;
  cout << "******************************************************************" <<endl;
  cout << "******************************************************************" <<endl;
  
  TStopwatch timer;
  timer.Start();
  
  cout << "*  Starting  S T R E S S  basic suite                            *" <<endl;
  cout << "******************************************************************" <<endl;
  
  gBenchmark->Start("StressRooFit");
  
  TestBasic1 test1(fref,writeRef,doVerbose) ;
  StatusPrint(1,"Basic1",test1.runTest());
  
  gBenchmark->Stop("StressRooFit");
  
  
  //Print table with results
  Bool_t UNIX = strcmp(gSystem->GetName(), "Unix") == 0;
  printf("******************************************************************\n");
  if (UNIX) {
    FILE *fp = gSystem->OpenPipe("uname -a", "r");
    Char_t line[60];
    fgets(line,60,fp); line[59] = 0;
    printf("*  %s\n",line);
    gSystem->ClosePipe(fp);
  } else {
    const Char_t *os = gSystem->Getenv("OS");
    if (!os) printf("*  Windows 95\n");
    else     printf("*  %s %s \n",os,gSystem->Getenv("PROCESSOR_IDENTIFIER"));
  }
  
  printf("******************************************************************\n");
  gBenchmark->Print("StressFit");
#ifdef __CINT__
  Double_t reftime = 86.34; //macbrun interpreted
#else
  Double_t reftime = 12.07; //macbrun compiled
#endif
  const Double_t rootmarks = 800*reftime/gBenchmark->GetCpuTime("StressRooFit");
  
  printf("******************************************************************\n");
  printf("*  ROOTMARKS =%6.1f   *  Root%-8s  %d/%d\n",rootmarks,gROOT->GetVersion(),
         gROOT->GetVersionDate(),gROOT->GetVersionTime());
  printf("******************************************************************\n");
  
  printf("Time at the end of job = %f seconds\n",timer.CpuTime());

  fref->Close() ;

  return 0;
}

//_____________________________batch only_____________________
#ifndef __CINT__

int main(int argc,const char *argv[]) 
{
  RooRandom::randomGenerator()->SetSeed(12345) ;

  Bool_t doWrite = kFALSE ;
  Int_t doVerbose = 0 ;

  // Parse command line arguments 
  for (Int_t i=1 ;  i<argc ; i++) {
    string arg = argv[i] ;
    if (arg=="-w") {
      cout << "stressRooFit: running in writing mode to updating reference file" << endl ;
      doWrite = kTRUE ;
    }

    if (arg=="-v") {
      cout << "stressRooFit: running in verbose mode" << endl ;
      doVerbose = 1 ;
    }

    if (arg=="-vv") {
      cout << "stressRooFit: running in very verbose mode" << endl ;
      doVerbose = 2 ;
    }
  }

  gBenchmark = new TBenchmark();
  stressRooFit("stressRooFit_ref.root",doWrite,doVerbose);  
  return 0;
}

#endif
