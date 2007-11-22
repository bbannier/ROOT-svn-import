// $Header: /soft/cvsroot/AliRoot/EVE/Reve/.SKEL.h,v 1.2 2006/10/12 14:08:45 mtadel Exp $

#ifndef REVE_CLASS_H
#define REVE_CLASS_H

#include <Reve/Reve.h>

#include <TObject.h>

namespace Reve {

class CLASS
{
private:
  CLASS(const CLASS&);            // Not implemented
  CLASS& operator=(const CLASS&); // Not implemented

protected:

public:
  CLASS();
  virtual ~CLASS() {}

  ClassDef(CLASS, 1);
}; // endclass CLASS

}

#endif
