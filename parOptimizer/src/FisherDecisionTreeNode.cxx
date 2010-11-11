// @(#)root/tmva $Id: FisherDecisionTreeNode.cxx 34800 2010-08-13 09:30:29Z hvoss $    
// Author: Andreas Hoecker, Joerg Stelzer, Helge Voss, Kai Voss, Eckhard von Toerne 

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : TMVA::FisherDecisionTreeNode                                                *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation of a Decision Tree Node                                    *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *      Kai Voss        <Kai.Voss@cern.ch>       - U. of Victoria, Canada         *
 *      Eckhard von Toerne <evt@physik.uni-bonn.de>  - U. of Bonn, Germany        *
 *                                                                                *
 * CopyRight (c) 2009:                                                            *
 *      CERN, Switzerland                                                         * 
 *      U. of Victoria, Canada                                                    * 
 *      MPI-K Heidelberg, Germany                                                 * 
*       U. of Bonn, Germany                                                       *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/
   
//_______________________________________________________________________
//                                                                      
// Node for the Decision Tree                                           
//
// The node specifies ONE variable out of the given set of selection variable
// that is used to split the sample which "arrives" at the node, into a left
// (background-enhanced) and a right (signal-enhanced) sample.
//_______________________________________________________________________

#include <algorithm>
#include <exception>
#include <iomanip>

#include "TMVA/MsgLogger.h"
#include "TMVA/FisherDecisionTreeNode.h"
#include "TMVA/Tools.h"
#include "TMVA/Event.h"

using std::string;

ClassImp(TMVA::FisherDecisionTreeNode)


//_______________________________________________________________________
TMVA::FisherDecisionTreeNode::FisherDecisionTreeNode()
: TMVA::DecisionTreeNode()
{
   // constructor of an essentially "empty" node floating in space
   if (!fgLogger) fgLogger = new TMVA::MsgLogger( "FisherDecisionTreeNode" );

   if (fgIsTraining){
      fTrainInfo = new DTNodeTrainingInfo();
      //std::cout << "Node constructor with TrainingINFO"<<std::endl;
   }
   else {
      //std::cout << "**Node constructor WITHOUT TrainingINFO"<<std::endl;
      fTrainInfo = 0;
   }
}

//_______________________________________________________________________
TMVA::FisherDecisionTreeNode::FisherDecisionTreeNode(TMVA::Node* p, char pos)
   : TMVA::DecisionTreeNode(p, pos)
{
   // constructor of a daughter node as a daughter of 'p'
   if (!fgLogger) fgLogger = new TMVA::MsgLogger( "FisherDecisionTreeNode" );

   if (fgIsTraining){
      fTrainInfo = new DTNodeTrainingInfo();
      //std::cout << "Node constructor with TrainingINFO"<<std::endl;
   }
   else {
      //std::cout << "**Node constructor WITHOUT TrainingINFO"<<std::endl;
      fTrainInfo = 0;
   }
}

//_______________________________________________________________________
TMVA::FisherDecisionTreeNode::FisherDecisionTreeNode(const TMVA::FisherDecisionTreeNode &n,
                                                     FisherDecisionTreeNode* parent)
   : TMVA::DecisionTreeNode(n)
{
   // copy constructor of a node. It will result in an explicit copy of
   // the node and recursively all it's daughters
   if (!fgLogger) fgLogger = new TMVA::MsgLogger( "FisherDecisionTreeNode" );

   this->SetParent( parent );
   if (n.GetLeft() == 0 ) this->SetLeft(NULL);
   else this->SetLeft( new FisherDecisionTreeNode( *((FisherDecisionTreeNode*)(n.GetLeft())),this));
   
   if (n.GetRight() == 0 ) this->SetRight(NULL);
   else this->SetRight( new FisherDecisionTreeNode( *((FisherDecisionTreeNode*)(n.GetRight())),this));
   
   if (fgIsTraining){
      fTrainInfo = new DTNodeTrainingInfo(*(n.fTrainInfo));
      //std::cout << "Node constructor with TrainingINFO"<<std::endl;
   }
   else {
      //std::cout << "**Node constructor WITHOUT TrainingINFO"<<std::endl;
      fTrainInfo = 0;
   }
}


//_______________________________________________________________________
Bool_t TMVA::FisherDecisionTreeNode::GoesRight(const TMVA::Event & e) const
{
   // test event if it decends the tree at this node to the right  

   Bool_t result;
   // first check if the fisher criterium is used or ordinary cuts:
   if (GetNFisherCoeff() == 0){
      
      result = (e.GetValue(this->GetSelector()) > this->GetCutValue() );

   }else{
      
      Double_t fisher = this->GetFisherCoeff(fFisherCoeff.size()-1); // the offset
      for (UInt_t ivar=0; ivar<fFisherCoeff.size()-1; ivar++)
         fisher += this->GetFisherCoeff(ivar)*(e.GetValue(ivar));

      result = fisher > this->GetCutValue();
   }

   if (fCutType == kTRUE) return result; //the cuts are selecting Signal ;
   else return !result;
   
}

//_______________________________________________________________________
Bool_t TMVA::FisherDecisionTreeNode::GoesLeft(const TMVA::Event & e) const
{
   // test event if it decends the tree at this node to the left 
   if (!this->GoesRight(e)) return kTRUE;
   else return kFALSE;
}


// print a node
//_______________________________________________________________________
void TMVA::FisherDecisionTreeNode::Print(ostream& os) const
{
   //print the node
   os << "< ***  "  << std::endl; 
   os << " d: "     << this->GetDepth()
      << std::setprecision(6)
      << "NCoef: "  << this->GetNFisherCoeff();
   for (Int_t i=0; i< (Int_t) this->GetNFisherCoeff(); i++) { os << "fC"<<i<<": " << this->GetFisherCoeff(i);}
   os << " ivar: "  << this->GetSelector()
      << " cut: "   << this->GetCutValue() 
      << " cType: " << this->GetCutType() 
      << " s: "     << this->GetNSigEvents()
      << " b: "     << this->GetNBkgEvents()
      << " nEv: "   << this->GetNEvents()
      << " suw: "   << this->GetNSigEvents_unweighted()
      << " buw: "   << this->GetNBkgEvents_unweighted()
      << " nEvuw: " << this->GetNEvents_unweighted()
      << " sepI: "  << this->GetSeparationIndex()
      << " sepG: "  << this->GetSeparationGain()
      << " nType: " << this->GetNodeType()
      << std::endl;
   
   os << "My address is " << long(this) << ", ";
   if (this->GetParent() != NULL) os << " parent at addr: "         << long(this->GetParent()) ;
   if (this->GetLeft()   != NULL) os << " left daughter at addr: "  << long(this->GetLeft());
   if (this->GetRight()  != NULL) os << " right daughter at addr: " << long(this->GetRight()) ;
   
   os << " **** > " << std::endl;
}

//_______________________________________________________________________
void TMVA::FisherDecisionTreeNode::PrintRec(ostream& os) const
{
   //recursively print the node and its daughters (--> print the 'tree')

   os << this->GetDepth() 
      << std::setprecision(6)
      << " "         << this->GetPos() 
      << "NCoef: "   << this->GetNFisherCoeff();
   for (Int_t i=0; i< (Int_t) this->GetNFisherCoeff(); i++) {os << "fC"<<i<<": " << this->GetFisherCoeff(i);}
   os << " ivar: "   << this->GetSelector()
      << " cut: "    << this->GetCutValue() 
      << " cType: "  << this->GetCutType() 
      << " s: "      << this->GetNSigEvents()
      << " b: "      << this->GetNBkgEvents()
      << " nEv: "    << this->GetNEvents()
      << " suw: "    << this->GetNSigEvents_unweighted()
      << " buw: "    << this->GetNBkgEvents_unweighted()
      << " nEvuw: "  << this->GetNEvents_unweighted()
      << " sepI: "   << this->GetSeparationIndex()
      << " sepG: "   << this->GetSeparationGain()
      << " res: "    << this->GetResponse()
      << " rms: "    << this->GetRMS()
      << " nType: "  << this->GetNodeType();
   if (this->GetCC() > 10000000000000.) os << " CC: " << 100000. << std::endl;
   else os << " CC: "  << this->GetCC() << std::endl;
  
   if (this->GetLeft()  != NULL) this->GetLeft() ->PrintRec(os);
   if (this->GetRight() != NULL) this->GetRight()->PrintRec(os);
}


//_______________________________________________________________________
void TMVA::FisherDecisionTreeNode::ReadAttributes(void* node, UInt_t /* tmva_Version_Code */  ) 
{   
   Float_t tempNSigEvents,tempNBkgEvents,tempNEvents,tempNSigEvents_unweighted,  tempNBkgEvents_unweighted,tempNEvents_unweighted, tempSeparationIndex, tempSeparationGain;  
   Double_t tempCC;

   Int_t nCoef;
   gTools().ReadAttr(node, "NCoef",  nCoef                  );
   this->SetNFisherCoeff(nCoef);
   Double_t tmp;
   for (Int_t i=0; i< (Int_t) this->GetNFisherCoeff(); i++) {
      gTools().ReadAttr(node, Form("fC%d",i),  tmp          );
      this->SetFisherCoeff(i,tmp);
   }
   gTools().ReadAttr(node, "IVar",  fSelector               );
   gTools().ReadAttr(node, "Cut",   fCutValue               );
   gTools().ReadAttr(node, "cType", fCutType                );
   // gTools().ReadAttr(node, "nS",    tempNSigEvents             );
   // gTools().ReadAttr(node, "nB",    tempNBkgEvents             );
   // gTools().ReadAttr(node, "nEv",   tempNEvents                );
   // gTools().ReadAttr(node, "nSuw",  tempNSigEvents_unweighted  );
   // gTools().ReadAttr(node, "nBuw",  tempNBkgEvents_unweighted  );
   // gTools().ReadAttr(node, "nEvuw", tempNEvents_unweighted     );
   // gTools().ReadAttr(node, "sepI",  tempSeparationIndex        );
   // gTools().ReadAttr(node, "sepG",  tempSeparationGain         );
   gTools().ReadAttr(node, "res",   fResponse               );
   gTools().ReadAttr(node, "rms",   fRMS                    );
   gTools().ReadAttr(node, "nType", fNodeType               );
   gTools().ReadAttr(node, "purity",fPurity                 );
   //   gTools().ReadAttr(node, "CC",    tempCC                  );
   // if (fTrainInfo){
   //    SetNSigEvents(tempNSigEvents);
   //    SetNBkgEvents(tempNBkgEvents);
   //    SetNEvents(tempNEvents);
   //    SetNSigEvents_unweighted(tempNSigEvents_unweighted);
   //    SetNBkgEvents_unweighted(tempNBkgEvents_unweighted);
   //    SetNEvents_unweighted(tempNEvents_unweighted);
   //    SetSeparationIndex(tempSeparationIndex);
   //    SetSeparationGain(tempSeparationGain);
   //    SetCC(tempCC);  
   // }
}
 

//_______________________________________________________________________
void TMVA::FisherDecisionTreeNode::AddAttributesToNode(void* node) const
{
   // add attribute to xml
   gTools().AddAttr(node, "NCoef", GetNFisherCoeff());
   for (Int_t i=0; i< (Int_t) this->GetNFisherCoeff(); i++) 
      gTools().AddAttr(node, Form("fC%d",i),  this->GetFisherCoeff(i));
   gTools().AddAttr(node, "IVar",   GetSelector());
   gTools().AddAttr(node, "Cut",   GetCutValue());
   gTools().AddAttr(node, "cType", GetCutType());
   // gTools().AddAttr(node, "nS",    GetNSigEvents());
   // gTools().AddAttr(node, "nB",    GetNBkgEvents());
   // gTools().AddAttr(node, "nEv",   GetNEvents());
   // gTools().AddAttr(node, "nSuw",  GetNSigEvents_unweighted());
   // gTools().AddAttr(node, "nBuw",  GetNBkgEvents_unweighted());
   // gTools().AddAttr(node, "nEvuw", GetNEvents_unweighted());
   // gTools().AddAttr(node, "sepI",  GetSeparationIndex());
   // gTools().AddAttr(node, "sepG",  GetSeparationGain());
   gTools().AddAttr(node, "res",   GetResponse());
   gTools().AddAttr(node, "rms",   GetRMS());
   gTools().AddAttr(node, "nType", GetNodeType());
   gTools().AddAttr(node, "purity",GetPurity());
   //   gTools().AddAttr(node, "CC",    (GetCC() > 10000000000000.)?100000.:GetCC());
   
}
//_______________________________________________________________________
void  TMVA::FisherDecisionTreeNode::SetFisherCoeff(Int_t ivar, Double_t coeff)
{
   // set fisher coefficients
   if ((Int_t) fFisherCoeff.size()<ivar+1) fFisherCoeff.resize(ivar+1) ; 
   fFisherCoeff[ivar]=coeff;      
}

//_______________________________________________________________________
void TMVA::FisherDecisionTreeNode::AddContentToNode( std::stringstream& /*s*/ ) const
{   
   // adding attributes to tree node  (well, was used in BinarySearchTree,
   // and somehow I guess someone programmed it such that we need this in
   // this tree too, although we don't..)
}

