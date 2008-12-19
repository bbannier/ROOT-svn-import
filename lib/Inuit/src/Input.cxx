// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Input.h"

using namespace Inuit;

#define ADD_ENUMCONST(ID, VALUE) #ID

const char* Input::GetETypeName() const { 
   static const char* sName[] = {
#include "Inuit/Enums/EType.h"
   };
   return sName[fType];
}

const char* Input::GetENonPrintableName() const {
   static const char* sName[] = {
#include "Inuit/Enums/ENonPrintable.h"
   };
   return sName[fNonPrintable];
}

const char* Input::GetEMouseName() const {
   static const char* sName[] = {
#include "Inuit/Enums/EMouse.h"
   };
   return sName[fMouse];
}

const char* Input::GetEWindowName() const {
   static const char* sName[] = {
#include "Inuit/Enums/EWindow.h"
   };
   return sName[fWindow];
}

void Input::GetEModifierName(std::string& name) const {
   static const char* sName[] = {
#include "Inuit/Enums/EModifiers.h"
   };
#undef ADD_ENUMCONST
#define ADD_ENUMCONST(ID, VALUE) ID
   static EModifiers sMod[] = {
#include "Inuit/Enums/EModifiers.h"
   };
   name = "";
   for (int i = 0; i < sizeof(sMod) / sizeof(char*); ++i)
      if (fModifiers & sMod[i]) { 
         if (!name.empty()) name += "|";
         name += sName[i];
      }
}

