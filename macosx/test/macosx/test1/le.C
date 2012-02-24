// Mainframe macro generated from application: /Users/tpochep/root_x11_un/bin/root.exe
// By ROOT version 5.33/01 on 2012-02-16 13:04:13

#ifndef ROOT_TGDockableFrame
#include "TGDockableFrame.h"
#endif
#ifndef ROOT_TGMenu
#include "TGMenu.h"
#endif
#ifndef ROOT_TGMdiDecorFrame
#include "TGMdiDecorFrame.h"
#endif
#ifndef ROOT_TG3DLine
#include "TG3DLine.h"
#endif
#ifndef ROOT_TGMdiFrame
#include "TGMdiFrame.h"
#endif
#ifndef ROOT_TGMdiMainFrame
#include "TGMdiMainFrame.h"
#endif
#ifndef ROOT_TGMdiMenu
#include "TGMdiMenu.h"
#endif
#ifndef ROOT_TGColorSelect
#include "TGColorSelect.h"
#endif
#ifndef ROOT_TGListBox
#include "TGListBox.h"
#endif
#ifndef ROOT_TGNumberEntry
#include "TGNumberEntry.h"
#endif
#ifndef ROOT_TGScrollBar
#include "TGScrollBar.h"
#endif
#ifndef ROOT_TGComboBox
#include "TGComboBox.h"
#endif
#ifndef ROOT_TGuiBldHintsEditor
#include "TGuiBldHintsEditor.h"
#endif
#ifndef ROOT_TGuiBldNameFrame
#include "TGuiBldNameFrame.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#ifndef ROOT_TGFileDialog
#include "TGFileDialog.h"
#endif
#ifndef ROOT_TGShutter
#include "TGShutter.h"
#endif
#ifndef ROOT_TGButtonGroup
#include "TGButtonGroup.h"
#endif
#ifndef ROOT_TGCanvas
#include "TGCanvas.h"
#endif
#ifndef ROOT_TGFSContainer
#include "TGFSContainer.h"
#endif
#ifndef ROOT_TGuiBldEditor
#include "TGuiBldEditor.h"
#endif
#ifndef ROOT_TGButton
#include "TGButton.h"
#endif
#ifndef ROOT_TGFSComboBox
#include "TGFSComboBox.h"
#endif
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif
#ifndef ROOT_TGMsgBox
#include "TGMsgBox.h"
#endif
#ifndef ROOT_TRootGuiBuilder
#include "TRootGuiBuilder.h"
#endif
#ifndef ROOT_TGTab
#include "TGTab.h"
#endif
#ifndef ROOT_TGListView
#include "TGListView.h"
#endif
#ifndef ROOT_TGSplitter
#include "TGSplitter.h"
#endif
#ifndef ROOT_TGStatusBar
#include "TGStatusBar.h"
#endif
#ifndef ROOT_TGListTree
#include "TGListTree.h"
#endif
#ifndef ROOT_TGuiBldGeometryFrame
#include "TGuiBldGeometryFrame.h"
#endif
#ifndef ROOT_TGToolTip
#include "TGToolTip.h"
#endif
#ifndef ROOT_TGToolBar
#include "TGToolBar.h"
#endif
#ifndef ROOT_TGuiBldDragManager
#include "TGuiBldDragManager.h"
#endif

#include "Riostream.h"

void le()
{

   // main frame
   TGMainFrame *fMainFrame1209 = new TGMainFrame(gClient->GetRoot(),10, 10, kMainFrame | kVerticalFrame);
   fMainFrame1209->AddInput(kEnterWindowMask | kLeaveWindowMask);   
   fMainFrame1209->SetName("fMainFrame1209");
   fMainFrame1209->SetLayoutBroken(kTRUE);

   ULong_t ucolor;        // will reflect user color changes
   gClient->GetColorByName("#85c2a3",ucolor);

   // horizontal frame
   TGHorizontalFrame *fHorizontalFrame615 = new TGHorizontalFrame(fMainFrame1209, 680, 528, kHorizontalFrame,ucolor);
   fHorizontalFrame615->AddInput(kEnterWindowMask | kLeaveWindowMask);
   
   fHorizontalFrame615->SetName("fHorizontalFrame615");
   fHorizontalFrame615->SetLayoutBroken(kTRUE);
   

   gClient->GetColorByName("#ff0000",ucolor);

   // horizontal frame
   TGHorizontalFrame *fHorizontalFrame685 = new TGHorizontalFrame(fHorizontalFrame615, 240, 232, kHorizontalFrame,ucolor);
   fHorizontalFrame685->AddInput(kEnterWindowMask | kLeaveWindowMask);
   
   fHorizontalFrame685->SetName("fHorizontalFrame685");
   
   fHorizontalFrame615->AddFrame(fHorizontalFrame685, new TGLayoutHints(kLHintsNormal));
   fHorizontalFrame685->MoveResize(24,136,240,232);
   

   gClient->GetColorByName("#7d99d1",ucolor);

   // horizontal frame
   TGHorizontalFrame *fHorizontalFrame747 = new TGHorizontalFrame(fHorizontalFrame615,240,240,kHorizontalFrame | kRaisedFrame, ucolor);
   fHorizontalFrame747->SetName("fHorizontalFrame747");
   fHorizontalFrame747->SetLayoutBroken(kTRUE);
   fHorizontalFrame747->AddInput(kEnterWindowMask | kLeaveWindowMask);

   fHorizontalFrame615->AddFrame(fHorizontalFrame747, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   fHorizontalFrame747->MoveResize(416,128,240,240);

   gClient->GetColorByName("#5954d9",ucolor);

   // horizontal frame
   TGHorizontalFrame *fHorizontalFrame756 = new TGHorizontalFrame(fHorizontalFrame615,240,120,kHorizontalFrame, ucolor);
   fHorizontalFrame756->SetName("fHorizontalFrame756");
   fHorizontalFrame756->SetLayoutBroken(kTRUE);
   fHorizontalFrame756->AddInput(kEnterWindowMask | kLeaveWindowMask);

   fHorizontalFrame615->AddFrame(fHorizontalFrame756, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fHorizontalFrame756->MoveResize(24,200,240,120);

   fMainFrame1209->AddFrame(fHorizontalFrame615, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fHorizontalFrame615->MoveResize(8,16,680,528);

   fMainFrame1209->Resize(697,552);
   fMainFrame1209->MapSubwindows();
   fMainFrame1209->Resize(fMainFrame1209->GetDefaultSize());
   fMainFrame1209->MapWindow();
}  
