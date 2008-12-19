// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_FRAME
#define INCLUDE_INUIT_FRAME

#include "Inuit/Widgets/Widget.h"
#include <list>
#include <algorithm>

namespace Inuit {
   class TerminalDriver;

   class Frame: public Widget {
   public:
      Frame(Frame* parent, Pos pos, Pos size):
         Widget(parent, pos), fSize(size), fFocusedElement(0) {}

      Pos GetSize() const { return fSize; }

      virtual TerminalDriver& GetTerminalDriver() const {
         return GetParent()->GetTerminalDriver(); }

      void AddElement(Widget* element) {
         if (fElements.empty())
            fFocusedElement = element;
         fElements.push_back(element);
      }

      void RemoveElement(Widget* element) {
         fElements.erase(std::find(fElements.begin(), fElements.end(), element));
         if (fFocusedElement == element)
            if (fElements.empty())
               fFocusedElement = 0;
            else {
               fFocusedElement = fElements.front();
               fFocusedElement->Draw();
            }
      }

      void Draw() {
         for (std::list<Widget*>::iterator iE = fElements.begin();
            iE != fElements.end(); ++iE)
            (*iE)->Draw();
      }

      Widget* GetFocusedElement() const { return fFocusedElement; }
      Widget* GetFocusedWidget() {
         return fFocusedElement->GetFocusedWidget();
      }

   private:
      Pos fSize;
      std::list<Widget*> fElements;
      Widget* fFocusedElement;
   };
}
#endif // INCLUDE_INUIT_FRAME
