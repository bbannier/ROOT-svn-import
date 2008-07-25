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
#include "ContainerNode.h"
#include "RWLock.h"

namespace Reflex {
   namespace Internal {
      namespace ContainerTools {
         //-------------------------------------------------------------------------------
         // A stack of nodes.
         // Used for keeping track of freed nodes for fast re-allocation.
         class Stack {
         public:
            // Create an empty stack
            Stack(): fSize(0) {}
            // Pop the topmost object
            Link* Pop() {
               --fSize;
               return fHead.RemoveAfter(0);
            }
            // Push an object onto the stack
            void Push(Link* e) {
               ++fSize;
               fHead.InsertAfter(0, e);
            }
            // Get the stack's number of elements
            size_t Size() const { return fSize; }
            // Whether the stack is non-empty:
            operator bool() const { return Size() != 0; }
         private:
            Link fHead;   // fHead ->...-> free1 -> free0, i.e. fHead is the most recently freed node
            size_t fSize; // number of elements in the stack
         };


         //-------------------------------------------------------------------------------
         // Arena management for nodes.
         // Implements creation and deletion of nodes in an efficient way, including
         // re-use, prevention of memory fragmentation, delayed deletion in case a
         // node is referenced.
         // Nodes are allocated en bloc as "pages". When all of a page's slots are used,
         // a new page gets allocated.
         class NodeArena {
         public:
            // Static node storage instance
            static NodeArena* Instance(int elementsize);
            // Set the maximum number of elements in a page
            static void SetElementsPerPage(size_t numElementsPerPage);

            // Free memory allocated by this node storage
            bool ReleaseInstance();

            // Number of elements managed by this node storage
            size_t Entries() const;

            Link* New();

            void Delete(Link* n) {
               // Mark a node as free, by adding it to fFree.
               REFLEX_RWLOCK_W(fLock);
               fFree.Push(n);
            }

         private:
            NodeArena(size_t elementsize);

            // Free memory used by the node storage
            ~NodeArena();

         private:
            char* fWatermark; // pointer to the next free slot
            std::list<char*> fPages; // pages managed by the node storage
            Stack  fFree; // stack of freed nodes
            size_t fElementSize; // size of the nodes
            mutable RWLock fLock; // Read / write lock
            static size_t fgElementsPerPage; // number of nodes per page
            static std::vector< std::list<NodeArena*> > fgInstances; // instances: index i contains a list of all node storages for nodes of size i
            static int fgDebug; // debug level
            static RWLock fgLock; // lock for instance management
         };

      } // namespace ContainerTools
   } // namespace Internal
} // namespace Reflex

#endif // Reflex_ContainerArena
