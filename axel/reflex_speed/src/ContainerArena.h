// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerArena
#define Reflex_ContainerArena

#include <list>
#include <vector>
#include <iostream>
#include <algorithm>

#include "RWLock.h"
#include "ContainerLink.h"

namespace Reflex {
   namespace Internal {
      namespace ContainerTools {
         class Link1Base;

         //-------------------------------------------------------------------------------
         // A stack of nodes.
         // Used for keeping track of freed nodes for fast re-allocation.
         class Stack {
         public:
            // Create an empty stack
            Stack(): fSize(0) {}
            // Pop the topmost object
            Link1Base* Pop() {
               --fSize;
               return const_cast<Link1Base*>(fHead.RemoveAfter());
            }
            // Push an object onto the stack
            void Push(Link1Base* e) {
               ++fSize;
               fHead.InsertAfter(e);
            }
            // Get the stack's number of elements
            size_t Size() const { return fSize; }
            // Whether the stack is non-empty:
            operator bool() const { return Size() != 0; }
         private:
            Link1Base fHead;   // fHead ->...-> free1 -> free0, i.e. fHead is the most recently freed node
            size_t fSize; // number of elements in the stack
         };


         //-------------------------------------------------------------------------------
         // Arena management for nodes.
         // Implements creation and deletion of nodes in an efficient way, including
         // re-use, prevention of memory fragmentation, delayed deletion in case a
         // node is referenced.
         // Nodes are allocated en bloc as "pages". When all of a page's slots are used,
         // a new page gets allocated.
         class NodeArenaBase {
         public:
            NodeArenaBase(size_t elementsize);

            // size of a node
            size_t ElementSize() const { return fElementSize; }
            // Number of elements managed by this node storage
            size_t Entries() const;

         protected:
            size_t fElementSize; // size of each element
            char* fWatermark; // pointer to the next free slot
            std::list<char*> fPages; // pages managed by the node storage
            static int fgDebug; // debug level
            static const size_t fgElementsPerPage; // number of nodes per page
         };


         //-------------------------------------------------------------------------------
         // Arena management for nodes.
         // Templated part of the arena management for a given LINKBASE type (Link1 or Link2)
         class NodeArena: public NodeArenaBase {
         public:
            // Static node storage instance
            static NodeArena* Instance(size_t elementsize);

            Link1Base* New();

            void Delete(Link1Base* n) {
               // Mark a node as free, by adding it to fFree.
               REFLEX_RWLOCK_W(fLock);
               fFree.Push(n);
               if (fFree.Size() == Entries())
                  ReleaseInstance();
            }

         private:
            NodeArena(size_t elementSize): NodeArenaBase(elementSize) {}
            // Free memory used by the node storage
            ~NodeArena();

            // Free memory allocated by this node storage
            bool ReleaseInstance();

         private:
            Stack fFree; // stack of freed nodes
            mutable RWLock fLock; // Read / write lock

            static std::vector< std::list<NodeArena*> > fgInstances; // instances: index i contains a list of all node storages for nodes of size i
            static RWLock fgLock; // lock for instance management
         };

      } // namespace ContainerTools
   } // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline
Reflex::Internal::ContainerTools::Link1Base*
Reflex::Internal::ContainerTools::NodeArena::New() {
//-------------------------------------------------------------------------------
// Create a new node.
// Take one from fFree, or allocate one if needed.
   REFLEX_RWLOCK_W(fLock);
   if (fFree) {
      return fFree.Pop();
   }
   char* n = fWatermark;
   fWatermark += fElementSize;
   if ((size_t)(fWatermark - fPages.back()) >= fgElementsPerPage * fElementSize) {
      fWatermark = new char[fgElementsPerPage * fElementSize];
      fPages.push_back(fWatermark);
   }
   return (Link1Base*)n;
}

#endif // Reflex_ContainerArena
