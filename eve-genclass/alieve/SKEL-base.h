// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#ifndef CLASS_H
#define CLASS_H

#include "TObject.h"

//______________________________________________________________________________
// Short description of CLASS
//

class CLASS : public TObject //, public TAttBBox
{
public:
  CLASS();
  virtual ~CLASS() {}

  // For TAttBBox:
  // virtual void ComputeBBox();
  // If painting is needed:
  // virtual void Paint(Option_t* option="");

protected:

private:
  CLASS(const CLASS&);            // Not implemented
  CLASS& operator=(const CLASS&); // Not implemented

  ClassDef(CLASS, 0); // Short description.
};

#endif
