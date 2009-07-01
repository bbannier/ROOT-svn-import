// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_CatalogImplBase
#define Reflex_CatalogImplBase

#include "Reflex/Callback.h"

#include <map>
#include <set>

namespace Reflex {
   namespace Internal {
      class CatalogImpl;

      template <class ELEM>
      class CatalogImplBase {
      public:

         CatalogImplBase() {}
         ~CatalogImplBase() {}

         // Callbacks
         void RegisterCallback(Callback& cb);
         void UnregisterCallback(Callback& cb);

      private:
         void NofifyAnonymous(NotifyInfo& ni);

         std::set<Callback*>      fAnonymousCallbacks[kNotifyNumTransitions]; // unnamed callbacks; array over transitions
         std::map<std::string, std::set<Callback*> > fOrphanedCallbacks; // named callbacks for unknown type
      };


      template <class ELEM>
      class CatalogImplBaseT: public CatalogImplBase {
      public:

         CatalogImplBaseT() {}
         ~CatalogImplBaseT() {}

         // return whether it's OK to proceed (i.e. !vetoed)
         bool AddPre(ELEM& type, const std::string& name);
         void AddPost(ELEM& type, const std::string& name);
         void RemovePre(ELEM& type, const std::string& name);
         void RemovePost(ELEM& type, const std::string& name);
      };
   }
}

//-------------------------------------------------------------------------------
inline bool
template<>
Reflex::Internal::CatalogImplBaseT<ELEM>::AddPre(ELEM& type, const std::string& name) {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   TypeName* tn = ByTypeName(name);
   if (tn) return tn->ThisType();
   return Dummy::Type();
}


#endif // Reflex_TypeCatalogImpl
