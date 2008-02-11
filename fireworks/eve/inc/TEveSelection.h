// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveSelection
#define ROOT_TEveSelection

#include "TEveElement.h"

#include <map>

class TEveSelection : public TEveElementList
{
private:
   TEveSelection(const TEveSelection&);            // Not implemented
   TEveSelection& operator=(const TEveSelection&); // Not implemented

protected:
   typedef std::map<TEveElement*, Set_t>           SelMap_t;
   typedef std::map<TEveElement*, Set_t>::iterator SelMap_i;

   Bool_t           fActive;

   SelMap_t         fImpliedSelected;

   Select_foo       fSelElement;
   ImplySelect_foo  fIncImpSelElement;
   ImplySelect_foo  fDecImpSelElement;

   void DoElementSelect  (SelMap_i entry);
   void DoElementUnselect(SelMap_i entry);

public:
   TEveSelection(const Text_t* n="TEveSelection", const Text_t* t="");
   virtual ~TEveSelection() {}

   void SetHighlightMode();

   virtual Bool_t AcceptElement(TEveElement* el);

   virtual void AddElement(TEveElement* el);
   virtual void RemoveElementLocal(TEveElement* el);
   virtual void RemoveElementsLocal();

   // Interface to make selection active/non-active.
   // Not used yet, assumed to be active all the time.
   virtual void ActivateSelection();
   virtual void DeactivateSelection();

   ClassDef(TEveSelection, 0); // Short description.
};

#endif
