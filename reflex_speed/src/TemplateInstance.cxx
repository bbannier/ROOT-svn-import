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

#include "TemplateInstance.h"

#include "Reflex/Builder/TypeBuilder.h"
#include "OwnedMember.h"


//-------------------------------------------------------------------------------
Reflex::Internal::TemplateInstance::TemplateInstance(const std::string & templateArguments):
   fTemplateArgumentsAdaptor(fTemplateArguments) {
//-------------------------------------------------------------------------------
// Create the dictionary info for a template instance.
   std::vector<std::string> templArgStrVec = Tools::GenTemplateArgVec(templateArguments);
   for (std::vector<std::string>::const_iterator it = templArgStrVec.begin();
        it != templArgStrVec.end(); ++it) {
      fTemplateArguments.push_back(TypeBuilder(it->c_str()));
   }
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::TemplateInstance::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the template instance.
   buf += "<";
   for (size_t i = 0; i < fTemplateArguments.size(); ++i) {
      if (i)
         buf += ",";
      fTemplateArguments[ i ].Name(buf, mod);
   }
   buf += ">";
   return buf;
}


