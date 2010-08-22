// @(#)root/tmva $Id: FisherDecisionTreeNode.h 34801 2010-08-13 09:30:57Z hvoss $    
// Author: Andreas Hoecker, Joerg Stelzer, Helge Voss, Kai Voss, Eckhard von Toerne

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : FisherDecisionTreeNode                                                *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Node for the Decision Tree using multivariate splits using Fisher         *
 *       discriminants                                                            *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *      Kai Voss        <Kai.Voss@cern.ch>       - U. of Victoria, Canada         *
 *      Eckhard von Toerne <evt@physik.uni-bonn.de>  - U. of Bonn, Germany        *
 *                                                                                *
 * Copyright (c) 2009:                                                            *
 *      CERN, Switzerland                                                         * 
 *      U. of Victoria, Canada                                                    * 
 *      MPI-K Heidelberg, Germany                                                 * 
*       U. of Bonn, Germany                                                       *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_FisherDecisionTreeNode
#define ROOT_TMVA_FisherDecisionTreeNode

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// FisherDecisionTreeNode                                               //
//                                                                      //
// Node for the Decision Tree  with mulitvariate (Fisher) splits        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMVA_Node
#include "TMVA/Node.h"
#endif

#ifndef ROOT_TMVA_DecisionTreeNode
#include "TMVA/DecisionTreeNode.h"
#endif

#ifndef ROOT_TMVA_Version
#include "TMVA/Version.h"
#endif

#include <iostream>
#include <vector>
#include <map>
namespace TMVA {

   class Event;
   class MsgLogger;

   class FisherDecisionTreeNode: public DecisionTreeNode {
    
   public:
    
      // constructor of an essentially "empty" node floating in space
      FisherDecisionTreeNode ();
      // constructor of a daughter node as a daughter of 'p'
      FisherDecisionTreeNode (Node* p, char pos); 
    
      // copy constructor 
      FisherDecisionTreeNode (const FisherDecisionTreeNode &n, FisherDecisionTreeNode* parent = NULL); 
    
      virtual ~FisherDecisionTreeNode(){}

      virtual Node* CreateNode() const { return new FisherDecisionTreeNode(); }
    
      // test event if it decends the tree at this node to the right  
      virtual Bool_t GoesRight( const Event & ) const;
    
      // test event if it decends the tree at this node to the left 
      virtual Bool_t GoesLeft ( const Event & ) const;
    
      inline void SetNFisherCoeff(Int_t nvars){fFisherCoeff.resize(nvars);}
      inline UInt_t GetNFisherCoeff() const { return fFisherCoeff.size();}
      // set fisher coefficients
      void SetFisherCoeff(Int_t ivar, Double_t coeff);      
      // get fisher coefficients
      Double_t GetFisherCoeff(Int_t ivar) const {return fFisherCoeff.at(ivar);}
      
      
      // set the cut value applied at this node 
      inline void  SetCutValue ( Float_t c ) { fCutValue  = c; }
      // return the cut value applied at this node
      inline Float_t GetCutValue ( void ) const { return fCutValue;  }
    
      // set true: if event variable > cutValue ==> signal , false otherwise
      void SetCutType( Bool_t t   ) { fCutType = t; }
      // return kTRUE: Cuts select signal, kFALSE: Cuts select bkg
      Bool_t GetCutType( void ) const { return fCutType; }
    
      // printout of the node
      virtual void Print( ostream& os ) const;
    
      // recursively print the node and its daughters (--> print the 'tree')
      virtual void PrintRec( ostream&  os ) const;

      // return pointer to the left/right daughter or parent node
      inline virtual FisherDecisionTreeNode* GetLeft( )  const { return dynamic_cast<FisherDecisionTreeNode*>(fLeft); }
      inline virtual FisherDecisionTreeNode* GetRight( ) const { return dynamic_cast<FisherDecisionTreeNode*>(fRight); }
      inline virtual FisherDecisionTreeNode* GetParent( )const { return dynamic_cast<FisherDecisionTreeNode*>(fParent); }

      // set pointer to the left/right daughter and parent node
      inline virtual void SetLeft  (Node* l) { fLeft   = dynamic_cast<FisherDecisionTreeNode*>(l);} 
      inline virtual void SetRight (Node* r) { fRight  = dynamic_cast<FisherDecisionTreeNode*>(r);} 
      inline virtual void SetParent(Node* p) { fParent = dynamic_cast<FisherDecisionTreeNode*>(p);} 



      virtual void AddAttributesToNode(void* node) const;
      virtual void AddContentToNode(std::stringstream& s) const;

   private:
      
      virtual void ReadAttributes(void* node, UInt_t tmva_Version_Code = TMVA_VERSION_CODE );

      mutable DTNodeTrainingInfo* fTrainInfo;

      std::vector<Double_t>       fFisherCoeff;    // the other fisher coeff (offset at the last element

      ClassDef(FisherDecisionTreeNode,0) // Node for the Decision Tree 
    
        };
} // namespace TMVA

#endif 
