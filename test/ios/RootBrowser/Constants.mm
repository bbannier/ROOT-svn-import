#include "Constants.h"

namespace ROOT_IOSBrowser {

/////////////Geometric constants for ROOTObjectController.
//'L' postfix is for landscape, 'P' is for portrait.
////Main view (self.view):

//X and Y are the same for portrait and landscape orientation.
const float viewX = 0.f;
const float viewY = 0.f;

//portrait - 768 x 1004 (20 Y pixels are taken by iOS).
const float viewWP = 768.f;
const float viewHP = 1004.f;

//landscape - 1024 x 748 (20 Y pixels are taken by iOS).
const float viewWL = 1024.f;
const float viewHL = 748.f;

////Scroll view:

//X and Y are the same for landscape and portrait.
const float scrollX = 0.f;
const float scrollY = 44.f;//Navigation bar height.

//portrait - 768 x 960 (44 Y pixels from parent are taken by navigation bar).
const float scrollWP = 768.f;
const float scrollHP = 960.f;

//landscape - 1024 x 704 (44 Y pixels from parent are taken by navigation bar).
const float scrollWL = 1024.f;
const float scrollHL = 704.f;

//Default pad's width and height,
//when not zoomed, without editor
//or with editor in landscape orientation.
const float padW = 600.f;
const float padH = 600.f;

//This is pad's width and height, when
//pad is not zoomed and editor is visible,
//device orientation is portrait.
const float padWSmall = 500.f;
const float padHSmall = 500.f;

const float padXNoEditorP = scrollWP / 2 - padW / 2;
const float padYNoEditorP = scrollHP / 2 - padH / 2;
const float padXNoEditorL = scrollWL / 2 - padW / 2;
const float padYNoEditorL = scrollHL / 2 - padH / 2;

//X and Y for pad (no zoom) with editor in portrait orientation:
const float padXWithEditorP = 20.f;
const float padYWithEditorP = scrollHP / 2 - padHSmall / 2;

//X and Y for pad (no zoom) with editor in landscape orientation:
const float padXWithEditorL = 100.f;
const float padYWithEditorL = scrollHL / 2 - padH / 2;

}
