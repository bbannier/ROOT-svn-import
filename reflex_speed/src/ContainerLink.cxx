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

#include "ContainerNode.h"
#include "ContainerArena.h"

//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerTools::Link1Base::InsertAfter(Link1Base* newnext) {
//-------------------------------------------------------------------------------
   // Insert a Link1 between this and the next element:
   // convert "this -> oldnext"
   // to "this -> newnext -> oldnext"
   if (newnext) newnext->SetNext(Next());
   SetNext(newnext);
}

//-------------------------------------------------------------------------------
const Reflex::Internal::ContainerTools::Link1Base*
Reflex::Internal::ContainerTools::Link1Base::RemoveAfter() {
//-------------------------------------------------------------------------------
   // convert "this -> oldnext -> newnext"
   // to "this -> newnext"
   const Link1Base* oldnext = Next();
   if (!oldnext) return 0;
   SetNext(oldnext->Next());
   return oldnext;
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerTools::Link2Base::InsertAfter(Link2Base* newnext) {
//-------------------------------------------------------------------------------
   // Insert a Link2 between this and the next element:
   // convert "this -> oldnext"
   // to "this -> newnext -> oldnext"
   Link2Base* oldnext = const_cast<Link2Base*>(Next());
   if (oldnext) oldnext->SetPrev(newnext);
   if (newnext) newnext->Set(this, oldnext);
   SetNext(newnext);
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::Link2Base*
Reflex::Internal::ContainerTools::Link2Base::RemoveAfter() {
//-------------------------------------------------------------------------------
   // convert "prev -> this -> oldnext -> newnext"
   // to "prev -> this -> newnext"
   const Link2Base* oldnext = Next();
   if (!oldnext) return 0;
   Link2Base* newnext = const_cast<Link2Base*>(oldnext->Next());
   if (newnext) newnext->SetPrev(this);
   SetNext(newnext);
   return const_cast<Link2Base*>(oldnext);
}


