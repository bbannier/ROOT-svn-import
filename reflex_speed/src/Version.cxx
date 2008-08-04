// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2008

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

#include "Reflex/Version.h"
#include <stdlib.h>

//-------------------------------------------------------------------------------
unsigned int
Reflex::APIVersion::Revision() {
//-------------------------------------------------------------------------------
   // Retrieve the subversion revision number
   // corresponding to the last change of the Reflex API.

   static unsigned int sRevision = 0;
   if (!sRevision) {
      sRevision = atol(REFLEX_API_REVISION);
   }
   return sRevision;
};

//-------------------------------------------------------------------------------
const std::string&
Reflex::APIVersion::Date() {
//-------------------------------------------------------------------------------
   // Return the date of the Reflex API's last change, formatted as
   // "2008-07-03 16:16:09 +0200 (Thu, 03 Jul 2008)"

   static std::string sDate;
   if (sDate.empty()) {
      sDate = REFLEX_API_DATE;
      sDate.erase(sDate.length() - 2, 2);
   }
   return sDate;
}

//-------------------------------------------------------------------------------
const std::string&
Reflex::APIVersion::HeadURL() {
//-------------------------------------------------------------------------------
   // Return the repository URL of the current Reflex API

   static std::string sHeadURL;
   if (sHeadURL.empty()) {
      sHeadURL = REFLEX_API_HEADURL;
      sHeadURL.erase(sHeadURL.length() - 2, 2);
   }
   return sHeadURL;
}

//-------------------------------------------------------------------------------
const std::string&
Reflex::APIVersion::Id() {
//-------------------------------------------------------------------------------
   // Return the subversion repository ID corresponding to 
   // the last change the currently used Reflex API, formatted as
   // "Kernel.h 24646 2008-07-03 14:16:09Z axel"

   static std::string sId;
   if (sId.empty()) {
      sId = REFLEX_API_ID;
      sId.erase(sId.length() - 2, 2);
   }
   return sId;
}
