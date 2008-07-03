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

#include "Reflex/internal/ContainerNode.h"
#include "Reflex/internal/ContainerArena.h"

//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerTools::Link::InsertAfter(const Link* prev, Link* newnext) {
//-------------------------------------------------------------------------------
   // Insert a link between this and the next element:
   // convert "prev -> this -> oldnext"
   // to "prev -> this -> newnext -> oldnext"
   Link* oldnext = Next(prev);
   if (oldnext) oldnext->Set(newnext, oldnext->Next(this));
   if (newnext) newnext->Set(this, oldnext);
   Set(prev, newnext);
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::Link*
Reflex::Internal::ContainerTools::Link::RemoveAfter(const Link* prev) {
//-------------------------------------------------------------------------------
   // convert "prev -> this -> oldnext -> newnext"
   // to "prev -> this -> newnext"
   Link* oldnext = Next(prev);
   if (!oldnext) return 0;
   Link* newnext = oldnext->Next(this);
   if (newnext) newnext->Set(this, newnext->Next(oldnext));
   Set(prev, newnext);
   return oldnext;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerTools::LinkIter::ArenaDelete(Link* link) const {
//-------------------------------------------------------------------------------
// Ask NodeArena to delete a link.

   fArena->Delete(link);
   // the container might have been deleted, leaving its
   // arena pages around because this node was still needing
   // it. Tell the arena to check for 
   fArena->ReleaseInstance();
}

