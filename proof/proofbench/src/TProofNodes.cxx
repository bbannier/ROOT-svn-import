// @(#)root/proofx:$Id$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofNode                                                           //
//                                                                      //
// PROOF worker node information                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofNodes.h"
#include "TProof.h"
#include "TList.h"

//ClassImp(TProofNodes)

//______________________________________________________________________________
TProofNodes::TProofNodes(TProof* proof)
:fProof(proof), fNodes(0)
{
   Build();
}

//______________________________________________________________________________
TProofNodes::~TProofNodes()
{
   if (fNodes){
      fNodes->Delete();
   }
}

//______________________________________________________________________________
void TProofNodes::Build()
{
   if (!fProof) return;

   if (fNodes){
      fNodes->Delete();
   }
   fNodes=new TList;
   fNodes->SetOwner(kTRUE);

   const TList* slaves=fProof->GetListOfSlaveInfos();
   TIter nxtslave(slaves);
   TSlaveInfo* si=0;
   TList* node = 0;

   while (si=(TSlaveInfo*)(nxtslave())){
      TSlaveInfo* si_copy=(TSlaveInfo*)(si->Clone());
      if (!(node = (TList *) fNodes->FindObject(si->GetName()))) {
         node = new TList;
         //si's are owned by the member fSlaveInfo of fProof
         node->SetOwner(kTRUE); 
         node->SetName(si_copy->GetName());
         node->Add(si_copy);
         fNodes->Add(node);
      } else {
         node->Add(si_copy);
      }
   }
}

//______________________________________________________________________________
Int_t TProofNodes::ActivateWorkers(const TString& workers)
{
   TString sworkers=workers.Strip(TString::kTrailing, 'x');
   Int_t nworkersnode=sworkers.Atoi();
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=(TList*)(nxtnode())){
      TIter nxtworker(node);
      TSlaveInfo* si=0;
      Int_t nactiveworkers=0;
      while (si=(TSlaveInfo*)(nxtworker())){
         const char* ordinal=si->GetOrdinal();
         if (nactiveworkers<nworkersnode){
            if (si->fStatus==TSlaveInfo::kActive){
            }
            else if (si->fStatus==TSlaveInfo::kNotActive){
               fProof->ActivateWorker(ordinal);
            }
            nactiveworkers++;
         }
         else{
            if (si->fStatus==TSlaveInfo::kActive){
               fProof->DeactivateWorker(ordinal);
            }
         }
      }
   }

   //Rebuild
   Build();

   //And check
   Int_t nactivetotal=0; //total number of active workers on the cluster
   TIter nxtnode2(fNodes);
   while (node=(TList*)(nxtnode2())){
      TIter nxtworker(node);
      TSlaveInfo* si=0;
      Int_t nactiveworkers=0;
      while (si=(TSlaveInfo*)(nxtworker())){
         if (si->fStatus==TSlaveInfo::kActive){
            nactiveworkers++;
         }
         else if (si->fStatus==TSlaveInfo::kNotActive){
         }
      }
      nactivetotal+=nactiveworkers;
      if (nactiveworkers!=nworkersnode){
         Warning("ActivateWorkers", "%d ( out of %d requested) workers "
                 "were activated on node %s",
                  nactiveworkers, nworkersnode, node->GetName());
      }
   }

   return nactivetotal;
}

//______________________________________________________________________________
Int_t TProofNodes::GetMaxNWorkers() const
{
   Int_t maxnworkers=0;
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=(TList*)(nxtnode())){
      Int_t nworkers=node->GetSize();
      maxnworkers=nworkers>maxnworkers?nworkers:maxnworkers;
   }
   return maxnworkers;
}

//______________________________________________________________________________
Int_t TProofNodes::GetNWorkersCluster() const
{
   Int_t nworkers=0;
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=(TList*)(nxtnode())){
      nworkers+=node->GetSize();
   }
   return nworkers;
}

//______________________________________________________________________________
TList* TProofNodes::GetListOfNodes() const
{
   return fNodes;
}

//______________________________________________________________________________
void TProofNodes::Print(Option_t* option) const
{
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=dynamic_cast<TList*>(nxtnode())){
     node->Print(option);
   }
}
