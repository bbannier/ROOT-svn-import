// @(#)root/tmva $Id$
// Author: Andreas Hoecker, Peter Speckmayer, Joerg Stelzer, Helge Voss

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : VariableTransformBase                                                 *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation (see header for description)                               *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Peter Speckmayer <Peter.Speckmayer@cern.ch>  - CERN, Switzerland          *
 *      Joerg Stelzer   <Joerg.Stelzer@cern.ch>  - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#include <iomanip>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <set>

#include "TMath.h"
#include "TVectorD.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

#include "TMVA/VariableTransformBase.h"
#include "TMVA/Ranking.h"
#include "TMVA/Config.h"
#include "TMVA/Tools.h"
#include "TMVA/Version.h"

#ifndef ROOT_TMVA_MsgLogger
#include "TMVA/MsgLogger.h"
#endif

ClassImp(TMVA::VariableTransformBase)

//_______________________________________________________________________
TMVA::VariableTransformBase::VariableTransformBase( DataSetInfo& dsi,
                                                    Types::EVariableTransform tf,
                                                    const TString& trfName )
   : TObject(),
     fDsi(dsi),
     fTransformedEvent(0),
     fBackTransformedEvent(0),
     fVariableTransform(tf),
     fEnabled( kTRUE ),
     fCreated( kFALSE ),
     fNormalise( kFALSE ),
     fTransformName(trfName),
     fTMVAVersion(TMVA_VERSION_CODE),
     fLogger( 0 )
{
   // standard constructor
   fLogger = new MsgLogger(this, kINFO);
   for (UInt_t ivar = 0; ivar < fDsi.GetNVariables(); ivar++) {
      fVariables.push_back( VariableInfo( fDsi.GetVariableInfo(ivar) ) );
   }
   for (UInt_t itgt = 0; itgt < fDsi.GetNTargets(); itgt++) {
      fTargets.push_back( VariableInfo( fDsi.GetTargetInfo(itgt) ) );
   }
}

//_______________________________________________________________________
TMVA::VariableTransformBase::~VariableTransformBase()
{
   if (fTransformedEvent!=0)     delete fTransformedEvent;
   if (fBackTransformedEvent!=0) delete fBackTransformedEvent;
   // destructor
   delete fLogger;
}

//_______________________________________________________________________
void TMVA::VariableTransformBase::SelectInput( const TString& _inputVariables  )
{
   // select the variables/targets/spectators which serve as input to the transformation
   TString inputVariables = _inputVariables;


   // unselect all variables first
   fGet.clear();       

   UInt_t nvars  = GetNVariables();
   UInt_t ntgts  = GetNTargets();
   UInt_t nspcts = GetNSpectators();

   typedef std::set<Int_t> SelectedIndices;

   SelectedIndices varIndices;
   SelectedIndices tgtIndices;
   SelectedIndices spctIndices;

   if (inputVariables == "") // default is all variables and all targets 
   {                         //   (the default can be changed by decorating this member function in the implementations)
      inputVariables = "_V_,_T_";
   }

   TList* inList = gTools().ParseFormatLine( inputVariables, "," );
   TListIter inIt(inList);
   while (TObjString* os = (TObjString*)inIt()) {
      TString variables = os->GetString();
      
      if( variables.BeginsWith("_") && variables.EndsWith("_") ) { // special symbol (keyword)
	 variables.Remove( 0,1); // remove first "_"
	 variables.Remove( variables.Length()-1,1 ); // remove last "_"

	 if( variables.BeginsWith("V") ) {       // variables
	    variables.Remove(0,1); // remove "V"
	    if( variables.Length() == 0 ){
	       for( UInt_t ivar = 0; ivar < nvars; ++ivar ) {
		  fGet.push_back( std::make_pair<Char_t,UInt_t>('v',ivar) );
		  varIndices.insert( ivar );
	       }
	    } else {
	       UInt_t idx = variables.Atoi();
	       if( idx >= nvars )
		  Log() << kFATAL << "You selected variable with index : " << idx << " of only " << nvars << " variables." << Endl;
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('v',idx) );
	       varIndices.insert( idx );
	    }
	 }else if( variables.BeginsWith("T") ) {       // targets
	    variables.Remove(0,1); // remove "T"
	    if( variables.Length() == 0 ){
	       for( UInt_t itgt = 0; itgt < ntgts; ++itgt ) {
		  fGet.push_back( std::make_pair<Char_t,UInt_t>('t',itgt) );
		  tgtIndices.insert( itgt );
	       }
	    } else {
	       UInt_t idx = variables.Atoi();
	       if( idx >= ntgts )
		  Log() << kFATAL << "You selected target with index : " << idx << " of only " << ntgts << " targets." << Endl;
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('t',idx) );
	       tgtIndices.insert( idx );
	    }
	 }else if( variables.BeginsWith("S") ) {       // spectators
	    variables.Remove(0,1); // remove "S"
	    if( variables.Length() == 0 ){
	       for( UInt_t ispct = 0; ispct < nspcts; ++ispct ) {
		  fGet.push_back( std::make_pair<Char_t,UInt_t>('s',ispct) );
		  spctIndices.insert( ispct );
	       }
	    } else {
	       UInt_t idx = variables.Atoi();
	       if( idx >= nspcts )
		  Log() << kFATAL << "You selected spectator with index : " << idx << " of only " << nspcts << " spectators." << Endl;
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('s',idx) );
	       spctIndices.insert( idx );
	    }
	 }
      }else{ // no keyword
	 for( UInt_t ivar = 0; ivar < nvars; ++ivar ) { // search all variables
	    if( fDsi.GetVariableInfo( ivar ).GetLabel() == variables ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('v',ivar) );
	       varIndices.insert( ivar );
	       break;
	    }
	 }
	 for( UInt_t itgt = 0; itgt < ntgts; ++itgt ) { // search all targets
	    if( fDsi.GetTargetInfo( itgt ).GetLabel() == variables ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('t',itgt) );
	       tgtIndices.insert( itgt );
	       break;
	    }
	 }
	 for( UInt_t ispct = 0; ispct < nspcts; ++ispct ) { // search all spectators
	    if( fDsi.GetSpectatorInfo( ispct ).GetLabel() == variables ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('s',ispct) );
	       spctIndices.insert( ispct );
	       break;
	    }
	 }
      }

   }


   for( SelectedIndices::iterator it = varIndices.begin(), itEnd = varIndices.end(); it != itEnd; ++it ) {
      Int_t idx = (*it);
      fPut.push_back( std::make_pair<Char_t,UInt_t>('v',idx) );
   }
   for( SelectedIndices::iterator it = tgtIndices.begin(), itEnd = tgtIndices.end(); it != itEnd; ++it ) {
      Int_t idx = (*it);
      fPut.push_back( std::make_pair<Char_t,UInt_t>('t',idx) );
   }
   for( SelectedIndices::iterator it = spctIndices.begin(), itEnd = spctIndices.end(); it != itEnd; ++it ) {
      Int_t idx = (*it);
      fPut.push_back( std::make_pair<Char_t,UInt_t>('s',idx) );
   }

   Log() << kINFO << "Transformation, Variable selection : " << Endl;

   
   ItVarTypeIdx itGet = fGet.begin(), itGetEnd = fGet.end();
   ItVarTypeIdx itPut = fPut.begin(), itPutEnd = fPut.end();
   for( ; itGet != itGetEnd; ++itGet ) {
      TString inputTypeString  = "?";

      Char_t inputType = (*itGet).first;
      Int_t inputIdx  = (*itGet).second;

      TString inputLabel = "NOT FOUND";
      if( inputType == 'v' ) {
	 inputLabel = fDsi.GetVariableInfo( inputIdx ).GetLabel();
	 inputTypeString = "variable";
      }
      else if( inputType == 't' ){
	 inputLabel = fDsi.GetTargetInfo( inputIdx ).GetLabel();
	 inputTypeString = "target";
      }
      else if( inputType == 's' ){
	 inputLabel = fDsi.GetSpectatorInfo( inputIdx ).GetLabel();
	 inputTypeString = "spectator";
      }

      TString outputTypeString  = "?";

      Char_t outputType = (*itPut).first;
      Int_t outputIdx  = (*itPut).second;

      TString outputLabel = "NOT FOUND";
      if( outputType == 'v' ) {
	 outputLabel = fDsi.GetVariableInfo( outputIdx ).GetLabel();
	 outputTypeString = "variable";
      }
      else if( outputType == 't' ){
	 outputLabel = fDsi.GetTargetInfo( outputIdx ).GetLabel();
	 outputTypeString = "target";
      }
      else if( outputType == 's' ){
	 outputLabel = fDsi.GetSpectatorInfo( outputIdx ).GetLabel();
	 outputTypeString = "spectator";
      }


      Log() << kINFO << "Input : " << inputTypeString.Data() << " '" << inputLabel.Data() << "' (index=" << inputIdx << ").   <---> "
	    <<          "Output : " << outputTypeString.Data() << " '" << outputLabel.Data() << "' (index=" << outputIdx << ")." << Endl;

      ++itPut;
   }

}


//_______________________________________________________________________
void TMVA::VariableTransformBase::GetInput( const Event* event, std::vector<Float_t>& input, Bool_t backTransformation ) const
{
   // select the values from the event

   ItVarTypeIdxConst itEntry;
   ItVarTypeIdxConst itEntryEnd;

   if( backTransformation ){
      itEntry = fPut.begin();
      itEntryEnd = fPut.end();
   }
   else {
      itEntry = fGet.begin();
      itEntryEnd = fGet.end();
   }

   input.clear();
   for( ; itEntry != itEntryEnd; ++itEntry ) {
      Char_t type = (*itEntry).first;
      Int_t  idx  = (*itEntry).second;

      switch( type ) {
      case 'v':
	 input.push_back( event->GetValue(idx) );
	 break;
      case 't':
	 input.push_back( event->GetTarget(idx) );
	 break;
      case 's':
	 input.push_back( event->GetSpectator(idx) );
	 break;
      default:
	 Log() << kFATAL << "VariableTransformBase/GetInput : unknown type '" << type << "'." << Endl;
      }
   }
}

//_______________________________________________________________________
void TMVA::VariableTransformBase::SetOutput( Event* event, std::vector<Float_t>& output, const Event* oldEvent, Bool_t backTransformation ) const
{
   // select the values from the event
   
   std::vector<Float_t>::iterator itOutput = output.begin();

   if( oldEvent )
      event->CopyVarValues( *oldEvent );

   try {

      ItVarTypeIdxConst itEntry;
      ItVarTypeIdxConst itEntryEnd;

      if( !backTransformation ){ // as in GetInput, but the other way round (from fPut for transformation, from fGet for backTransformation)
	 itEntry = fPut.begin();
	 itEntryEnd = fPut.end();
      }
      else {
	 itEntry = fGet.begin();
	 itEntryEnd = fGet.end();
      }


      for( ; itEntry != itEntryEnd; ++itEntry ) {
	 Char_t type = (*itEntry).first;
	 Int_t  idx  = (*itEntry).second;
	 
	 Float_t value = (*itOutput);

	 switch( type ) {
	 case 'v':
	    event->SetVal( idx, value );
	    break;
	 case 't':
	    event->SetTarget( idx, value );
	    break;
	 case 's':
	    event->SetSpectator( idx, value );
	    break;
	 default:
	    Log() << kFATAL << "VariableTransformBase/GetInput : unknown type '" << type << "'." << Endl;
	 }
	 ++itOutput;
      }
   }catch( std::exception& except ){
      Log() << kFATAL << "VariableTransformBase/SetOutput : exception/" << except.what() << Endl;
      throw;
   }
}


//_______________________________________________________________________
void TMVA::VariableTransformBase::CountVariableTypes( UInt_t& nvars, UInt_t& ntgts, UInt_t& nspcts )
{
   // count variables, targets and spectators
   nvars = ntgts = nspcts = 0;

   for( ItVarTypeIdxConst itEntry = fGet.begin(), itEntryEnd = fGet.end(); itEntry != itEntryEnd; ++itEntry ) {
	 Char_t type = (*itEntry).first;

	 switch( type ) {
	 case 'v':
	    nvars++;
	    break;
	 case 't':
	    ntgts++;
	    break;
	 case 's':
	    nspcts++;
	    break;
	 default:
	    Log() << kFATAL << "VariableTransformBase/GetVariableTypeNumbers : unknown type '" << type << "'." << Endl;
	 }
   }
}


//_______________________________________________________________________
void TMVA::VariableTransformBase::CalcNorm( const std::vector<Event*>& events ) 
{
   // TODO --> adapt to variable,target,spectator selection

   // method to calculate minimum, maximum, mean, and RMS for all
   // variables used in the MVA

   if (!IsCreated()) return;

   const UInt_t nvars = GetNVariables();
   const UInt_t ntgts = GetNTargets();

   UInt_t nevts = events.size();

   TVectorD x2( nvars+ntgts ); x2 *= 0;
   TVectorD x0( nvars+ntgts ); x0 *= 0;   

   Double_t sumOfWeights = 0;
   for (UInt_t ievt=0; ievt<nevts; ievt++) {
      const Event* ev = events[ievt];

      Double_t weight = ev->GetWeight();
      sumOfWeights += weight;
      for (UInt_t ivar=0; ivar<nvars; ivar++) {
         Double_t x = ev->GetValue(ivar);
         if (ievt==0) {
            Variables().at(ivar).SetMin(x);
            Variables().at(ivar).SetMax(x);
         } 
         else {
            UpdateNorm( ivar,  x );
         }
         x0(ivar) += x*weight;
         x2(ivar) += x*x*weight;
      }
      for (UInt_t itgt=0; itgt<ntgts; itgt++) {
         Double_t x = ev->GetTarget(itgt);
         if (ievt==0) {
            Targets().at(itgt).SetMin(x);
            Targets().at(itgt).SetMax(x);
         } 
         else {
            UpdateNorm( nvars+itgt,  x );
         }
         x0(nvars+itgt) += x*weight;
         x2(nvars+itgt) += x*x*weight;
      }
   }

   if (sumOfWeights <= 0) {
      Log() << kFATAL << " the sum of event weights calcualted for your input is == 0"
            << " or exactly: " << sumOfWeights << " there is obviously some problem..."<< Endl;
   } 

   // set Mean and RMS
   for (UInt_t ivar=0; ivar<nvars; ivar++) {
      Double_t mean = x0(ivar)/sumOfWeights;
      
      Variables().at(ivar).SetMean( mean ); 
      if (x2(ivar)/sumOfWeights - mean*mean < 0) {
         Log() << kFATAL << " the RMS of your input variable " << ivar 
               << " evaluates to an imaginary number: sqrt("<< x2(ivar)/sumOfWeights - mean*mean
               <<") .. sometimes related to a problem with outliers and negative event weights"
               << Endl;
      }
      Variables().at(ivar).SetRMS( TMath::Sqrt( x2(ivar)/sumOfWeights - mean*mean) );
   }
   for (UInt_t itgt=0; itgt<ntgts; itgt++) {
      Double_t mean = x0(nvars+itgt)/sumOfWeights;
      Targets().at(itgt).SetMean( mean ); 
      if (x2(nvars+itgt)/sumOfWeights - mean*mean < 0) {
         Log() << kFATAL << " the RMS of your target variable " << itgt 
               << " evaluates to an imaginary number: sqrt(" << x2(nvars+itgt)/sumOfWeights - mean*mean
               <<") .. sometimes related to a problem with outliers and negative event weights"
               << Endl;
      }
      Targets().at(itgt).SetRMS( TMath::Sqrt( x2(nvars+itgt)/sumOfWeights - mean*mean) );
   }

   Log() << kVERBOSE << "Set minNorm/maxNorm for variables to: " << Endl;
   Log() << std::setprecision(3);
   for (UInt_t ivar=0; ivar<GetNVariables(); ivar++)
      Log() << "    " << Variables().at(ivar).GetInternalName()
              << "\t: [" << Variables().at(ivar).GetMin() << "\t, " << Variables().at(ivar).GetMax() << "\t] " << Endl;
   Log() << kVERBOSE << "Set minNorm/maxNorm for targets to: " << Endl;
   Log() << std::setprecision(3);
   for (UInt_t itgt=0; itgt<GetNTargets(); itgt++)
      Log() << "    " << Targets().at(itgt).GetInternalName()
              << "\t: [" << Targets().at(itgt).GetMin() << "\t, " << Targets().at(itgt).GetMax() << "\t] " << Endl;
   Log() << std::setprecision(5); // reset to better value       
}

//_______________________________________________________________________
std::vector<TString>* TMVA::VariableTransformBase::GetTransformationStrings( Int_t /*cls*/ ) const
{ 
   // TODO --> adapt to variable,target,spectator selection

   // default transformation output
   // --> only indicate that transformation occurred
   std::vector<TString>* strVec = new std::vector<TString>;
   for (UInt_t ivar=0; ivar<GetNVariables(); ivar++) {
      strVec->push_back( Variables()[ivar].GetLabel() + "_[transformed]");
   }

   return strVec;   
}

//_______________________________________________________________________
void TMVA::VariableTransformBase::UpdateNorm ( Int_t ivar,  Double_t x ) 
{
   // TODO --> adapt to variable,target,spectator selection

   // update min and max of a given variable (target) and a given transformation method
   Int_t nvars = fDsi.GetNVariables();
   if( ivar < nvars ){
      if (x < Variables().at(ivar).GetMin()) Variables().at(ivar).SetMin(x);
      if (x > Variables().at(ivar).GetMax()) Variables().at(ivar).SetMax(x);
   }else{
      if (x < Targets().at(ivar-nvars).GetMin()) Targets().at(ivar-nvars).SetMin(x);
      if (x > Targets().at(ivar-nvars).GetMax()) Targets().at(ivar-nvars).SetMax(x);
   }
}



//_______________________________________________________________________
void TMVA::VariableTransformBase::AttachXMLTo(void* parent) 
{
   // create XML description the transformation (write out info of selected variables)

   void* selxml = gTools().AddChild(parent, "Selection");

   void* inpxml = gTools().AddChild(selxml, "Input");
   gTools().AddAttr(inpxml, "NInputs", fGet.size() );

   for( ItVarTypeIdx itGet = fGet.begin(), itGetEnd = fGet.end(); itGet != itGetEnd; ++itGet ) {
      UInt_t idx  = (*itGet).second;
      Char_t type = (*itGet).first;
      
      TString label = "";
      TString typeString = "";
      switch( type ){
      case 'v':
	 typeString = "Variable";
	 label = fDsi.GetVariableInfo( idx ).GetLabel();
	 break;
      case 't':
	 typeString = "Target";
	 label = fDsi.GetTargetInfo( idx ).GetLabel();
	 break;
      case 's':
	 typeString = "Spectator";
	 label = fDsi.GetSpectatorInfo( idx ).GetLabel();
	 break;
      default:
	 Log() << kFATAL << "VariableTransformBase/AttachXMLTo unknown variable type '" << type << "'." << Endl;
      }
	 
       void* idxxml = gTools().AddChild(inpxml, "Input");
//      gTools().AddAttr(idxxml, "Index", idx);
      gTools().AddAttr(idxxml, "Type",  typeString);
      gTools().AddAttr(idxxml, "Label", label);
   }


   void* outxml = gTools().AddChild(parent, "Output");
   gTools().AddAttr(outxml, "NOutputs", fPut.size() );

   for( ItVarTypeIdx itPut = fPut.begin(), itPutEnd = fPut.end(); itPut != itPutEnd; ++itPut ) {
      UInt_t idx  = (*itPut).second;
      Char_t type = (*itPut).first;
      
      TString label = "";
      TString typeString = "";
      switch( type ){
      case 'v':
	 typeString = "Variable";
	 label = fDsi.GetVariableInfo( idx ).GetLabel();
	 break;
      case 't':
	 typeString = "Target";
	 label = fDsi.GetTargetInfo( idx ).GetLabel();
	 break;
      case 's':
	 typeString = "Spectator";
	 label = fDsi.GetSpectatorInfo( idx ).GetLabel();
	 break;
      default:
	 Log() << kFATAL << "VariableTransformBase/AttachXMLTo unknown variable type '" << type << "'." << Endl;
      }
	 
       void* idxxml = gTools().AddChild(outxml, "Output");
//      gTools().AddAttr(idxxml, "Index", idx);
      gTools().AddAttr(idxxml, "Type",  typeString);
      gTools().AddAttr(idxxml, "Label", label);
   }


}

//_______________________________________________________________________
void TMVA::VariableTransformBase::ReadFromXML( void* selnode ) 
{
   // Read the input variables from the XML node

   void* inpnode = gTools().GetChild( selnode );
   void* outnode = gTools().GetChild( selnode );

   UInt_t nvars  = GetNVariables();
   UInt_t ntgts  = GetNTargets();
   UInt_t nspcts = GetNSpectators();



   // read inputs
   fGet.clear();       

   UInt_t nInputs = 0;
   gTools().ReadAttr(inpnode, "NInputs", nInputs);

   void* ch = gTools().GetChild( inpnode );
   while(ch) {
      TString typeString = "";
      TString label      = "";

      gTools().ReadAttr(ch, "Type",  typeString);
      gTools().ReadAttr(ch, "Label", label);
   
      if( typeString == "Variable"  ){
	 for( UInt_t ivar = 0; ivar < nvars; ++ivar ) { // search all variables
	    if( fDsi.GetVariableInfo( ivar ).GetLabel() == label ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('v',ivar) );
	       break;
	    }
	 }
      }else if( typeString == "Target"    ){
	 for( UInt_t itgt = 0; itgt < ntgts; ++itgt ) { // search all targets
	    if( fDsi.GetTargetInfo( itgt ).GetLabel() == label ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('t',itgt) );
	       break;
	    }
	 }
      }else if( typeString == "Spectator" ){
	 for( UInt_t ispct = 0; ispct < nspcts; ++ispct ) { // search all spectators
	    if( fDsi.GetSpectatorInfo( ispct ).GetLabel() == label ) {
	       fGet.push_back( std::make_pair<Char_t,UInt_t>('s',ispct) );
	       break;
	    }
	 }
      }else{
	 Log() << kFATAL << "VariableTransformationBase/ReadFromXML : unknown type '" << typeString << "'." << Endl;
      }
      ch = gTools().GetNextChild( ch );
   }

   // read outputs
   fPut.clear();       

   UInt_t nOutputs = 0;
   gTools().ReadAttr(outnode, "NOutputs", nOutputs);

   void* chOut = gTools().GetChild( outnode );
   while(chOut) {
      TString typeString = "";
      TString label      = "";

      gTools().ReadAttr(chOut, "Type",  typeString);
      gTools().ReadAttr(chOut, "Label", label);
   
      if( typeString == "Variable"  ){
	 for( UInt_t ivar = 0; ivar < nvars; ++ivar ) { // search all variables
	    if( fDsi.GetVariableInfo( ivar ).GetLabel() == label ) {
	       fPut.push_back( std::make_pair<Char_t,UInt_t>('v',ivar) );
	       break;
	    }
	 }
      }else if( typeString == "Target"    ){
	 for( UInt_t itgt = 0; itgt < ntgts; ++itgt ) { // search all targets
	    if( fDsi.GetTargetInfo( itgt ).GetLabel() == label ) {
	       fPut.push_back( std::make_pair<Char_t,UInt_t>('t',itgt) );
	       break;
	    }
	 }
      }else if( typeString == "Spectator" ){
	 for( UInt_t ispct = 0; ispct < nspcts; ++ispct ) { // search all spectators
	    if( fDsi.GetSpectatorInfo( ispct ).GetLabel() == label ) {
	       fPut.push_back( std::make_pair<Char_t,UInt_t>('s',ispct) );
	       break;
	    }
	 }
      }else{
	 Log() << kFATAL << "VariableTransformationBase/ReadFromXML : unknown type '" << typeString << "'." << Endl;
      }
      chOut = gTools().GetNextChild( chOut );
   }


}


//_______________________________________________________________________
void TMVA::VariableTransformBase::MakeFunction( std::ostream& /*fout*/, const TString& /*fncName*/, Int_t part,
						UInt_t /*trCounter*/, Int_t /*cls*/ )
{
   // getinput and setoutput equivalent
   if( part == 0 ){ // getinput equivalent
//       fout << std::endl;
//       fout << "   std::vector<double> input; " << std::endl;
//    // select the values from the event
//    input.clear();
//    for( ItVarTypeIdxConst itEntry = fGet.begin(), itEntryEnd = fGet.end(); itEntry != itEntryEnd; ++itEntry ) {
//       Char_t type = (*itEntry).first;
//       Int_t  idx  = (*itEntry).second;

//       switch( type ) {
//       case 'v':
// 	 input.push_back( event->GetValue(idx) );
// 	 break;
//       case 't':
// 	 input.push_back( event->GetTarget(idx) );
// 	 break;
//       case 's':
// 	 input.push_back( event->GetSpectator(idx) );
// 	 break;
//       default:
// 	 Log() << kFATAL << "VariableTransformBase/GetInput : unknown type '" << type << "'." << Endl;
//       }
//    }


   }else if( part == 1){ // setoutput equivalent
   }
}

