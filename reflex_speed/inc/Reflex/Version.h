// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Version
#define Reflex_Version

#include <string>

// Incremented when the version is changed.
// Versions > 100000 are on branches
#define REFLEX_API_VERSION 100001

// Keywords corresponding to the above revision
#define REFLEX_API_REVISION_RAW "$Revision: 24646 $"
#define REFLEX_API_DATE_RAW     "$Date: 2008-07-03 16:16:09 +0200 (Thu, 03 Jul 2008) $"
#define REFLEX_API_HEADURL_RAW  "$HeadURL: https://root.cern.ch/svn/root/branches/dev/axel/reflex_speed/inc/Reflex/Kernel.h $"
#define REFLEX_API_ID_RAW       "$Id$"

// User readable version of the keywords:
// they do not include the tags themselves but only their values
// (and a trailing '$')
#define REFLEX_API_REVISION     (REFLEX_API_REVISION_RAW + 11)
#define REFLEX_API_DATE         (REFLEX_API_DATE_RAW + 7)
#define REFLEX_API_HEADURL      (REFLEX_API_HEADURL_RAW + 10)
#define REFLEX_API_ID           (REFLEX_API_ID_RAW + 5)

namespace Reflex {
   namespace APIVersion {
      unsigned int Revision();
      const std::string& Date();
      const std::string& HeadURL();
      const std::string& Id();
   }
}

#endif // Reflex_Version

