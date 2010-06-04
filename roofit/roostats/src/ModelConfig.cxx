// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "RooStats/ModelConfig.h"

#include "TROOT.h"

#ifndef ROO_MSG_SERVICE
#include "RooMsgService.h"
#endif


namespace RooStats {

ModelConfig::~ModelConfig() { 
   // destructor.
//    if( fOwnsWorkspace && fWS) { 
//       std::cout << "ModelConfig : delete own workspace " << std::endl;
//       delete fWS;
//    }
}

void ModelConfig::SetWorkspace(RooWorkspace & ws) {
   // set a workspace that owns all the necessary components for the analysis
   if (!fWS) { 
      fWS = &ws;
      fWSName = ws.GetName();
      fRefWS = &ws; 
   }   
   else{
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
      fWS->merge(ws);
      RooMsgService::instance().setGlobalKillBelow(RooFit::DEBUG) ;
   }
   
}

const RooWorkspace * ModelConfig::GetWS() const { 
   // get workspace if pointer is null get from the TRef 
   if (fWS) return fWS; 
   // get from TRef
   fWS = dynamic_cast<RooWorkspace *>(fRefWS.GetObject() );
   if (fWS) return fWS; 
   coutE(ObjectHandling) << "workspace not set" << endl;
   return 0;
}

void ModelConfig::SetSnapshot(const RooArgSet& set) {
   // save snaphot in the workspace 
   // and use values passed with the set
   if (!fWS) {
      coutE(ObjectHandling) << "workspace not set" << endl;
      return;
   }
   fSnapshotName = GetName();
   if (fSnapshotName.size()  > 0) fSnapshotName += "_";
   fSnapshotName += set.GetName();
   if (fSnapshotName.size()  > 0) fSnapshotName += "_";
   fSnapshotName += "snapshot";
   fWS->saveSnapshot(fSnapshotName.c_str(), set, false);  // import also the given parameter values 
   // define the set also in WS
   DefineSetInWS(fSnapshotName.c_str(), set); 
}    

const RooArgSet * ModelConfig::GetSnapshot() const{
   // load the snapshot from ws and return the corresponding set with the snapshot values
   if (!fWS) return 0; 
   if (!fWS->loadSnapshot(fSnapshotName.c_str()) ) return 0; 
   return fWS->set(fSnapshotName.c_str() );
}


void ModelConfig::DefineSetInWS(const char* name, const RooArgSet& set) {
   // helper functions to avoid code duplication
   if (!fWS) {
      coutE(ObjectHandling) << "workspace not set" << endl;
      return;
   }
   if (! fWS->set( name )){
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
      // use option to import missing constituents
      // if content with same name exist they will not be imported ? 

      fWS->defineSet(name, set,true);  

      RooMsgService::instance().setGlobalKillBelow(RooFit::DEBUG) ;
   }
}
   
void ModelConfig::ImportPdfInWS(const RooAbsPdf & pdf) { 
   // internal function to import Pdf in WS
   if (!fWS) { 
      coutE(ObjectHandling) << "workspace not set" << endl;
      return;
   }
   if (! fWS->pdf( pdf.GetName() ) ){
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
      fWS->import(pdf);
      RooMsgService::instance().setGlobalKillBelow(RooFit::DEBUG) ;
   }
}
   
void ModelConfig::ImportDataInWS(RooAbsData & data) { 
   // internal function to import data in WS
   if (!fWS) {
      coutE(ObjectHandling) << "workspace not set" << endl;
      return;
   }
   if (! fWS->data( data.GetName() ) ){
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
      fWS->import(data);
      RooMsgService::instance().setGlobalKillBelow(RooFit::DEBUG) ;
   }
}


} // end namespace RooStats
