// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Widgets/Widget.h"
#include "Inuit/Widgets/Frame.h"

using namespace Inuit;

bool Widget::HandleEvent(const Event& event) {
   return GetParent()->HandleEvent(event);
}
