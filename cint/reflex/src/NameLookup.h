// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2006

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_NameLookup
#define Reflex_NameLookup

// Include files
#include "Reflex/Member.h"
#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include <cstddef>
#include <string>
#include <vector>
#include <set>

namespace Reflex {
   // forward declarations
   class Dictionary;

   // TODO: Point of declaration (3.3.1 [basic.scope.pdecl]) is not taken into account.

   class NameLookup {
private:
   // Data Members.
      const std::string fLookupName; // we're looking for a type / member of this name
      std::size_t fPosNamePart; // Start position in fLookupName of name currently being looked up.
      std::size_t fPosNamePartLen; // Length of name currently being looked up.
      std::set<Scope> fLookedAtUsingDir; // Set of using-directive-nominated scopes already considered.
      Scope fCurrentScope; // Scope where lookup is currently occuring.
      const Dictionary fDictionary; // Dictionary into which to search
      bool fPartialSuccess; // Flag indicating that a left-prefix has matched.

      // intentionally not implemented:
      NameLookup(const NameLookup&);
      NameLookup& operator=(const NameLookup&);
       
private:
   // Internal Utility Functions.
      NameLookup(const std::string& name, const Scope& current, const Dictionary& dictionary); // We can only be created by our static func members.
      template <class T> T Lookup(bool isTemplateExpanded = false);
      template <class T> T LookupInScope();
      template <class T> T LookupInUnknownScope();
      void FindNextScopePos(); // Utility function, advance to the next name part in a qualified id.

public:
   // Public Interface.
      // 1. Lookup
      // @param dictionary: the dictionary into which to search, by default the dictionary of current()
      // @pre current == Scope::ByName(current.Name(), dictionary)
   static Type LookupType(const std::string& nam,
                          const Scope& current);
   static Type LookupType(const std::string& nam,
                          const Scope& current,
                          const Dictionary& dictionary);
   static Scope LookupScope(const std::string& nam,
                            const Scope& current);
   static Scope LookupScope(const std::string& nam,
                            const Scope& current,
                            const Dictionary& dictionary);
   static Member LookupMember(const std::string& nam,
                              const Scope& current);
   static Member LookupMember(const std::string& nam,
                              const Scope& current,
                              const Dictionary& dictionary);
   static Member LookupMemberUnqualified(const std::string& nam,
                                         const Scope& current);
   static Member LookupMemberUnqualified(const std::string& nam,
                                         const Scope& current,
                                         const Dictionary& dictionary);
   static Member LookupMemberQualified(const std::string& nam,
                                       const Dictionary& dictionary = DictionaryExe());

      // 2. OverloadResolution
   static Member OverloadResultion(const std::string& nam,
                                   const std::vector<Member>& funcs);

      // 3. AccessControl
   static Type AccessControl(const Type& typ,
                             const Scope& current);
   };

} // namespace Reflex

#endif // Reflex_NameLookup
