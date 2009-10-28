// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-27

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TModuleDoc.h"

//______________________________________________________________________________
//
// BEGIN_HTML
// <p>Represents modules of the documented product. Modules are sub-groups of
// classes of a product, which get separate index pages and user-provided
// documentation. For ROOT, a module is a sub-directory; it often corresponds 
// to a library. TModuleDoc, for example, is part of the HTML module,
// which is documented <a href="./HTML_index.html">here</a>. The list of
// all modules is shown e.g. in the <a href="ClassIndex.html">class index</a>.</p>
// <p>A module's documentation is searched by combining its source directory
// and the module documentation search path defined by 
// <a href="./THtml.html#THtml:SetModuleDocPath">THtml::SetModuleDocPath()</a>;
// it defaults to "../doc", i.e. for a module's sources in "module/src" its
// documentation is searched in "module/doc".
// END_HTML
//______________________________________________________________________________


ClassImp(Doc::TModuleDocInfo);
