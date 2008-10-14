// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "ClassTemplateInstance.h"

#include "Reflex/Scope.h"

#include "TemplateInstance.h"
#include "TypeTemplateImpl.h"
#include "Reflex/Tools.h"

#include <vector>
#include <string>
#include <sstream>

//-------------------------------------------------------------------------------
Reflex::Internal::ClassTemplateInstance::
ClassTemplateInstance(const char * typ, 
                      size_t size, 
                      const std::type_info & ti,
                      const Catalog& catalog,
                      unsigned int modifiers)
   : Class(typ, 
            size, 
            ti, 
            catalog,
            modifiers,
            kETTypeTemplateInstance),
     TemplateInstance(Tools::GetTemplateArguments(typ)),
     fTemplateFamily(TypeTemplate()) {
//-------------------------------------------------------------------------------
// Construct a template class instance dictionary information. This constructor
// takes case of deducing the template parameter names and generates the info for
// a template family if necessary.
   Scope s = DeclaringScope();

   std::string templateName = Tools::GetTemplateName(typ);

//    for (size_t i = 0; i < s.SubTypeTemplateSize(); ++i) {
//       TypeTemplate ttl = s.SubTypeTemplateAt(i);
//       if (ttl.Name(kScoped) == templateName) {
//          fTemplateFamily = ttl;
//          break;
//       }
//    }

   fTemplateFamily = TypeTemplate::ByName(templateName, fTemplateArguments.size());

   if (! fTemplateFamily) {
      std::vector < std::string > parameterNames(fTemplateArguments.size());
      for (size_t i = 65; i < 65 + fTemplateArguments.size(); ++i) {
         parameterNames.push_back("typename " + char(i));      
      }
      TypeTemplateImpl * tti = new TypeTemplateImpl(templateName.c_str(),
                                                     s,
                                                     parameterNames);
      fTemplateFamily = tti->ThisTypeTemplate();
      s.AddSubTypeTemplate(fTemplateFamily);
   }
  
   fTemplateFamily.AddTemplateInstance(TypeBase::operator Type());
}

