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
//constructor

   Build();
}

//______________________________________________________________________________
TProofNodes::~TProofNodes()
{
//destructor

   if (fNodes){
      fNodes->Delete();
   }
}

//______________________________________________________________________________
void TProofNodes::Build()
{
//Desctiption: Build the node list, which is a list of nodes whose members
//             in turn are lists of workers on the node.
//Input: Nothing
//Return: Nothing

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
//Description: Activate the same number of workers on all nodes.
//Input: workers: string of the form "nx" where non-negative integer n
//                is the number of worker on each node to be activated.
//Return: The number of active workers per node when the operation is
//        successful.
//        <0 otherwise.

   //Make sure worker list is up-to-date
   Build();

   TString sworkers=workers.Strip(TString::kTrailing, 'x');
   Int_t nworkersnode=sworkers.Atoi();
   Int_t ret=nworkersnode;
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
         ret=-1;
      }
   }

   return ret;
}

//______________________________________________________________________________
Int_t TProofNodes::GetMaxNWorkersANode() const
{
//Description: Maximum number of workers on nodes in the cluster.
//Input: None
//Return: Mamimum number of workers on nodes in the cluster. 

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
//Description: Get total number of workers in the cluster, either active
//             or inactive.
//Input: None.
//Return: Total number of workers in the cluster

   Int_t nworkers=0;
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=(TList*)(nxtnode())){
      nworkers+=node->GetSize();
   }
   return nworkers;
}

//______________________________________________________________________________
Int_t TProofNodes::GetNNodes() const
{
   //Description: Get number of nodes in the cluster
   //Input:: None.
   //return: Number of nodes in the cluster
   return fNodes->GetSize(); 
}

//______________________________________________________________________________
Int_t TProofNodes::GetMinNWorkersANode() const
{
   //Description: Get minumum number of workers on nodes in the cluster
   //Input:: None.
   //return: Minimum number of workers on nodes in the cluster
   Int_t minnworkers=9999999;
   Int_t nworkers=0;
   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=(TList*)(nxtnode())){
      nworkers=node->GetSize();
      minnworkers=(minnworkers>nworkers)?nworkers:minnworkers;
   }
   return minnworkers;
}

//______________________________________________________________________________
Int_t TProofNodes::GetNActives() const
{
   //Description: Get the number of active workers in the cluster

   TProofNodes* const localthis=(TProofNodes* const) this;
   localthis->Build();

   Int_t nactives=0;
   TIter nxtnode(fNodes);
   TList* node=0;

   while (node=(TList*)(nxtnode())){
      TIter nxtworker(node);
      TSlaveInfo* si=0;
      while (si=(TSlaveInfo*)(nxtworker())){
         if (si->fStatus==TSlaveInfo::kActive){
            nactives++;
         }
         else if (si->fStatus==TSlaveInfo::kNotActive){
         }
      }
   }
   return nactives;
}

//______________________________________________________________________________
TList* TProofNodes::GetListOfNodes() const
{
//Description: Get list of nodes.
//Input: None
//Return: List of nodes.

   return fNodes;
}

//______________________________________________________________________________
void TProofNodes::Print(Option_t* option) const
{
//Description: Print node list.

   TIter nxtnode(fNodes);
   TList* node=0;
   while (node=dynamic_cast<TList*>(nxtnode())){
     node->Print(option);
   }
}
