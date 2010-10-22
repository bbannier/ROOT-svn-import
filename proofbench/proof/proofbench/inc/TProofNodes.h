// @(#)root/proofx:$Id:$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofNodes
#define ROOT_TProofNodes

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofNodes                                                          //
//                                                                      //
// PROOF worker nodes information                                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class TProof;
class TList;

class TProofNodes: public TObject
{
private:
   TProof* fProof;
   TList* fNodes;   //List of node infos

   void Build();
public:
   TProofNodes(TProof* proof);

   virtual ~TProofNodes();
   Int_t ActivateWorkers(const TString& workers);
   Int_t GetMaxNWorkersANode() const;
   Int_t GetNWorkersCluster() const;
   Int_t GetNNodes() const;
   Int_t GetMinNWorkersANode() const;
   Int_t TProofNodes::GetNActives() const;
   TList* GetListOfNodes() const;
   void Print(Option_t* option="") const;
   //ClassDef(TProofNodes, 0) //Node and worker information
};

#endif
