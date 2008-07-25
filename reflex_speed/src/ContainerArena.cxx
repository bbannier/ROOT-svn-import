// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2007, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "ContainerArena.h"
#include <iostream>
#include <algorithm>

//-------------------------------------------------------------------------------
size_t Reflex::Internal::ContainerTools::NodeArena::fgElementsPerPage = 1000;

//-------------------------------------------------------------------------------
std::vector< std::list<Reflex::Internal::ContainerTools::NodeArena*> >
   Reflex::Internal::ContainerTools::NodeArena::fgInstances(256);

//-------------------------------------------------------------------------------
int Reflex::Internal::ContainerTools::NodeArena::fgDebug = 0;

//-------------------------------------------------------------------------------
Reflex::Internal::RWLock
Reflex::Internal::ContainerTools::NodeArena::fgLock;

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::NodeArena::NodeArena(size_t elementsize):
//-------------------------------------------------------------------------------
fElementSize(elementsize) {
   // Create a node storage, characterized by the nodes' size.
   // Allocate an initial page.
   fWatermark = new char[fgElementsPerPage * fElementSize];
   fPages.push_back(fWatermark);
}


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::NodeArena::~NodeArena() {
//-------------------------------------------------------------------------------
// Free the buffer allocated by the pages of this node arena.
   REFLEX_RWLOCK_W(fLock);
   char* buf;
   for (std::list<char*>::iterator iC = fPages.begin();
        iC != fPages.end(); ++iC) {
      buf = *iC;
      delete [] buf;
   }
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::ContainerTools::NodeArena::Entries() const {
//-------------------------------------------------------------------------------
// Return the number of allocated (i.e. filled) entries.
// Must be locked by caller!

   int entries = fPages.size() - 1; // last one only filled up to fWatermark
   entries *= fgElementsPerPage;
   entries += (fWatermark - fPages.back()) / fElementSize;
   return entries;
}


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::NodeArena*
Reflex::Internal::ContainerTools::NodeArena::Instance(int elementsize) {
//-------------------------------------------------------------------------------
// Return the instance of NodeArena for a given elementsize
   int index = elementsize;
   index -= sizeof(Link); // we always store that, so factor it out
   index -= 1; // and the object is expected to use at least one byte!
   bool needresize = false;
   bool neednew = false;
   {
      REFLEX_RWLOCK_R(fgLock);
      needresize = (index >= fgInstances.size());
      if (!needresize)
         neednew = (fgInstances[index].empty() || fgInstances[index].back()->fPages.size() > 1);
      else neednew = true;
   }
   if (neednew) {
      REFLEX_RWLOCK_W(fgLock);
      if (needresize)
         fgInstances.resize(index + 1);
      NodeArena* ret = new NodeArena(elementsize);
      fgInstances[index].push_back(ret);
      return ret;
   } else {
      REFLEX_RWLOCK_R(fgLock);
      return fgInstances[index].back();
   }
   return 0; // never executed - silences compiler warnings
}


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::Link*
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
   if (fWatermark - fPages.back() >= fgElementsPerPage * fElementSize) {
      fWatermark = new char[fgElementsPerPage * fElementSize];
      fPages.push_back(fWatermark);
   }
   return (Link*)n;
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::ContainerTools::NodeArena::ReleaseInstance() {
//-------------------------------------------------------------------------------
// Remove the memory used by the instance specified by elementsize.
// The memory is only freed if all nodes are deleted, i.e. if fFree
// contains as many entries as fPages can hold. Returns true in case
// the memory was freed.
   REFLEX_RWLOCK_W(fLock);
   REFLEX_RWLOCK_W(fgLock);
   int index = fElementSize;
   index -= sizeof(Link); // we always store that, so factor it out
   index -= 1; // and the object is expected to use at least one byte!
   if (index < fgInstances.size()) {
      std::list<NodeArena*>::iterator iNA
         = std::find(fgInstances[index].begin(), fgInstances[index].end(), this);
      if (fgDebug > 0) {
         int instances = 0;
         for (int i = 0; i < fgInstances.size(); ++i)
            instances += fgInstances[i].size();

         std::cout << "NodeArena DEBUG:  Releasing with..." << std::endl
              << "  pages: " << fPages.size() << std::endl
              << "  page size: " << fgElementsPerPage << std::endl
              << "  last page's used entries: " << (fWatermark - fPages.back()) / fElementSize << std::endl
              << "  fFree's size: " << fFree.Size() << std::endl
              << "  RESULT: " << ((fFree.Size() == Entries()) ? "SUCCESS" : "FAILED") << std::endl
              << "  number of node arena instances: " << instances << std::endl;
      }
      if (fFree.Size() == Entries()) {
         fgInstances[index].erase(iNA);
         delete this;
      }
      return true;
   } else
      std::cerr << "Reflex: ContainerBase::NodeArea::ReleaseInstance: " << std::endl
                << "  no instance with node size == " << fElementSize << "!" << std::endl;
   return false;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerTools::NodeArena::SetElementsPerPage(size_t numElementsPerPage) {
//-------------------------------------------------------------------------------
   REFLEX_RWLOCK_W(fgLock);
   fgElementsPerPage = numElementsPerPage;
}
