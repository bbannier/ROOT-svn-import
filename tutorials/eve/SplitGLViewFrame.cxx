
#include "TApplication.h"
#include "TSystem.h"
#include "TGFrame.h"
#include "TGLayout.h"
#include "TGSplitter.h"
#include "TGLWidget.h"
#include "TRootEmbeddedCanvas.h"
#include "TCanvas.h"
#include "TEvePad.h"
#include "TGeoManager.h"
#include "TString.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGFileDialog.h"
#include "TGMsgBox.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "HelpText.h"
#include "TClass.h"
#include "Riostream.h"

#include "TGPicture.h"
#include "TImage.h"
#include "TEnv.h"

#include "TFormula.h"
#include "TF1.h"
#include "TH1F.h"
#include "TGSplitFrame.h"
#include "TGLEmbeddedViewer.h"
#include "TGShapedFrame.h"

#ifdef WIN32
#include <TWin32SplashThread.h>
#endif

class SplitGLViewFrame : public TGMainFrame {

public:
   enum EMyCommands {
      kFileOpen, kFileExit, kHelpAbout,
      kGLPerspYOZ, kGLPerspXOZ, kGLPerspXOY,
      kGLXOY, kGLXOZ, kGLZOY,
      kGLOrthoRotate, kGLOrthoDolly,
      kSceneUpdate, kSceneUpdateAll
   };

private:
   TEvePad           *fPad;         // pad used as geometry container
   TGSplitFrame      *fSplitFrame;  // main (first) split frame
   TGLEmbeddedViewer *fTLViewer;    // top-left GL viewer
   TGLEmbeddedViewer *fTRViewer;    // top-right GL viewer
   TGLEmbeddedViewer *fBLViewer;    // bottom-left GL viewer
   TGLEmbeddedViewer *fBRViewer;    // bottom-right GL viewer
   TGLEmbeddedViewer *fActViewer;   // actual (active) GL viewer
   TGMenuBar         *fMenuBar;     // main menu bar
   TGPopupMenu       *fMenuFile;    // 'File' popup menu
   TGPopupMenu       *fMenuHelp;    // 'Help' popup menu
   TGPopupMenu       *fMenuCamera;  // 'Camera' popup menu
   TGPopupMenu       *fMenuScene;   // 'Scene' popup menu
   TGStatusBar       *fStatusBar;   // status bar
   TGShapedToolTip   *fShapedToolTip;   // shaped tooltip

public:
   SplitGLViewFrame(const TGWindow *p=0, UInt_t w = 800, UInt_t h = 600);
   virtual ~SplitGLViewFrame();

   void           HandleMenu(Int_t id);
   void           OnMouseIdle(TGLPhysicalShape *shape, UInt_t posx, UInt_t posy);
   void           OnMouseOver(TGLPhysicalShape *shape);
   void           OnViewerActivated();
   void           OpenFile(const char *fname);
   void           ToggleOrthoRotate();
   void           ToggleOrthoDolly();

   ClassDef(SplitGLViewFrame, 0)
};

const char *filetypes[] = { 
   "ROOT files",    "*.root",
   "All files",     "*",
   0,               0 
};

ClassImp(SplitGLViewFrame)

//______________________________________________________________________________
SplitGLViewFrame::SplitGLViewFrame(const TGWindow *p, UInt_t w, UInt_t h) :
   TGMainFrame(p, w, h), fActViewer(0), fShapedToolTip(0)

{
   // Main frame constructor.

   TGSplitFrame *frm;

   // create the "file" popup menu
   fMenuFile = new TGPopupMenu(fClient->GetRoot());
   fMenuFile->AddEntry("&Open...", kFileOpen);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", kFileExit);

   // create the "camera" popup menu
   fMenuCamera = new TGPopupMenu(fClient->GetRoot());
   fMenuCamera->AddEntry("Perspective (Floor XOZ)", kGLPerspXOZ);
   fMenuCamera->AddEntry("Perspective (Floor YOZ)", kGLPerspYOZ);
   fMenuCamera->AddEntry("Perspective (Floor XOY)", kGLPerspXOY);
   fMenuCamera->AddEntry("Orthographic (XOY)", kGLXOY);
   fMenuCamera->AddEntry("Orthographic (XOZ)", kGLXOZ);
   fMenuCamera->AddEntry("Orthographic (ZOY)", kGLZOY);
   fMenuCamera->AddSeparator();
   fMenuCamera->AddEntry("Ortho allow rotate", kGLOrthoRotate);
   fMenuCamera->AddEntry("Ortho allow dolly",  kGLOrthoDolly);

   fMenuScene = new TGPopupMenu(fClient->GetRoot());
   fMenuScene->AddEntry("&Update Current", kSceneUpdate);
   fMenuScene->AddEntry("Update &All", kSceneUpdateAll);

   // create the "help" popup menu
   fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp->AddEntry("&About", kHelpAbout);

   // create the main menu bar
   fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fMenuFile, new TGLayoutHints(kLHintsTop | 
                      kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Camera", fMenuCamera, new TGLayoutHints(kLHintsTop | 
                      kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Scene", fMenuScene, new TGLayoutHints(kLHintsTop | 
                      kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Help", fMenuHelp, new TGLayoutHints(kLHintsTop | 
                      kLHintsRight));

   AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

   // connect menu signals to our menu handler slot
   fMenuFile->Connect("Activated(Int_t)", "SplitGLViewFrame", this,
                      "HandleMenu(Int_t)");
   fMenuCamera->Connect("Activated(Int_t)", "SplitGLViewFrame", this,
                        "HandleMenu(Int_t)");
   fMenuScene->Connect("Activated(Int_t)", "SplitGLViewFrame", this,
                       "HandleMenu(Int_t)");
   fMenuHelp->Connect("Activated(Int_t)", "SplitGLViewFrame", this,
                      "HandleMenu(Int_t)");
   // create the status bar
   Int_t parts[] = {45, 15, 10, 30};
   fStatusBar = new TGStatusBar(this, 50, 10);
   fStatusBar->SetParts(parts, 4);
   AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 
            0, 0, 10, 0));

   // create the split frames
   fSplitFrame = new TGSplitFrame(this, 800, 600);
   AddFrame(fSplitFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   // split it once
   fSplitFrame->HSplit();
   // then split each part again (this will make four parts)
   fSplitFrame->GetFirst()->VSplit();
   fSplitFrame->GetSecond()->VSplit();

   // create eve pad (our geometry container)
   fPad = new TEvePad();
   // get top left split frame
   frm = fSplitFrame->GetFirst()->GetFirst();
   // create (embed) a GL viewer inside
   fTLViewer = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fTLViewer->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to orthographic (XOY) for this viewer
   fTLViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   // connect signal we are interested to
   fTLViewer->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLViewFrame", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fTLViewer->Connect("Activated()", "SplitGLViewFrame", this, 
                      "OnViewerActivated()");
   fTLViewer->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLViewFrame", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");

   // get top right split frame
   frm = fSplitFrame->GetFirst()->GetSecond();
   // create (embed) a GL viewer inside
   fTRViewer = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fTRViewer->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to orthographic (XOZ) for this viewer
   fTRViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
   // connect signal we are interested to
   fTRViewer->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLViewFrame", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fTRViewer->Connect("Activated()", "SplitGLViewFrame", this, 
                      "OnViewerActivated()");
   fTRViewer->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLViewFrame", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");

   // get bottom left split frame
   frm = fSplitFrame->GetSecond()->GetFirst();
   // create (embed) a GL viewer inside
   fBLViewer = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fBLViewer->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to orthographic (ZOY) for this viewer
   fBLViewer->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
   // connect signal we are interested to
   fBLViewer->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLViewFrame", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fBLViewer->Connect("Activated()", "SplitGLViewFrame", this, 
                      "OnViewerActivated()");
   fBLViewer->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLViewFrame", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");

   // get bottom right split frame
   frm = fSplitFrame->GetSecond()->GetSecond();
   // create (embed) a GL viewer inside
   fBRViewer = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fBRViewer->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to perspective (XOZ) for this viewer
   fBRViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
   // connect signal we are interested to
   fBRViewer->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLViewFrame", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fBRViewer->Connect("Activated()", "SplitGLViewFrame", this, 
                      "OnViewerActivated()");
   fBRViewer->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLViewFrame", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");

   fShapedToolTip = new TGShapedToolTip("Default.png", 120, 22, 160, 110, 
                                        23, 115, 12, "#ffff80");
   Resize(GetDefaultSize());
   MapSubwindows();
   MapWindow();
}

//______________________________________________________________________________
SplitGLViewFrame::~SplitGLViewFrame()
{
   // Clean up main frame...
   //Cleanup();
   
   delete fShapedToolTip;
   delete fMenuFile;
   delete fMenuScene;
   delete fMenuCamera;
   delete fMenuHelp;
   delete fMenuBar;
   delete fStatusBar;
   delete fTLViewer;
   delete fTRViewer;
   delete fBLViewer;
   delete fBRViewer;
   delete fSplitFrame;
   delete fPad;
   gApplication->Terminate(0);
}

//______________________________________________________________________________
void SplitGLViewFrame::HandleMenu(Int_t id)
{
   // Handle menu items.

   switch (id) {

      case kFileOpen:
         {
            static TString dir(".");
            TGFileInfo fi;
            fi.fFileTypes = filetypes;
            fi.fIniDir    = StrDup(dir);
            new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
            if (fi.fFilename)
               OpenFile(fi.fFilename);
            dir = fi.fIniDir;
         }
         break;

      case kFileExit:
         CloseWindow();
         break;

      case kGLPerspYOZ:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
         break;
      case kGLPerspXOZ:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
         break;
      case kGLPerspXOY:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
         break;
      case kGLXOY:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
         break;
      case kGLXOZ:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
         break;
      case kGLZOY:
         if (fActViewer)
            fActViewer->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
         break;
      case kGLOrthoRotate:
         ToggleOrthoRotate();
         break;
      case kGLOrthoDolly:
         ToggleOrthoDolly();
         break;

      case kSceneUpdate:
         if (fActViewer)
            fActViewer->UpdateScene();
         break;

      case kSceneUpdateAll:
         fTLViewer->UpdateScene();
         fTRViewer->UpdateScene();
         fBLViewer->UpdateScene();
         fBRViewer->UpdateScene();
         break;

      case kHelpAbout:
         {
#ifdef R__UNIX
            TString rootx;
# ifdef ROOTBINDIR
            rootx = ROOTBINDIR;
# else
            rootx = gSystem->Getenv("ROOTSYS");
            if (!rootx.IsNull()) rootx += "/bin";
# endif
            rootx += "/root -a &";
            gSystem->Exec(rootx);
#else
#ifdef WIN32
            new TWin32SplashThread(kTRUE);
#else
            char str[32];
            sprintf(str, "About ROOT %s...", gROOT->GetVersion());
            hd = new TRootHelpDialog(this, str, 600, 400);
            hd->SetText(gHelpAbout);
            hd->Popup();
#endif
#endif
         }
         break;

      default:
         break;
   }
}

//______________________________________________________________________________
void SplitGLViewFrame::OnMouseIdle(TGLPhysicalShape *shape, UInt_t posx, UInt_t posy)
{
   // Slot used to handle "OnMouseIdle" signal coming from any GL viewer.
   // We receive a pointer on the physical shape in which the mouse cursor is
   // and the actual cursor position (x,y)

   Window_t wtarget;
   Int_t    x = 0, y = 0;

   static TH1F *h1f = 0;
   TFormula *form1 = new TFormula("form1","abs(sin(x)/x)");
   TF1 *sqroot = new TF1("sqroot","x*gaus(0) + [3]*form1",0,10);
   sqroot->SetParameters(10,4,1,20);
   if (h1f == 0)
      h1f = new TH1F("h1f","",50,0,10);
   h1f->Reset();
   h1f->SetFillColor(45);
   h1f->SetStats(0);
   h1f->FillRandom("sqroot",200);

   if (fShapedToolTip) {
      fShapedToolTip->UnmapWindow();
   }
   if (shape && shape->GetLogical() && shape->GetLogical()->GetExternal()) {
      // get the actual viewer who actually emitted the signal
      TGLEmbeddedViewer *actViewer = (TGLEmbeddedViewer *)gTQSender;
      // then translate coordinates from the root (screen) coordinates 
      // to the actual frame (viewer) ones
      gVirtualX->TranslateCoordinates(actViewer->GetFrame()->GetId(),
               fClient->GetDefaultRoot()->GetId(), posx, posy, x, y,
               wtarget);
      // Then display our tooltip at this x,y location
      if (fShapedToolTip) {
         fShapedToolTip->Show(x+5, y+5, Form("%s\n     \n%s",
                              shape->GetLogical()->GetExternal()->IsA()->GetName(), 
                              shape->GetLogical()->GetExternal()->GetName()), h1f);
      }
   }
}

//______________________________________________________________________________
void SplitGLViewFrame::OnMouseOver(TGLPhysicalShape *shape)
{
   // Slot used to handle "OnMouseOver" signal coming from any GL viewer.
   // We receive a pointer on the physical shape in which the mouse cursor is.

   // display informations on the physical shape in the status bar
   if (shape && shape->GetLogical() && shape->GetLogical()->GetExternal())
      fStatusBar->SetText(Form("Mouse Over: \"%s\"", 
         shape->GetLogical()->GetExternal()->GetName()), 0);
   else
      fStatusBar->SetText("", 0);
}

//______________________________________________________________________________
void SplitGLViewFrame::OnViewerActivated()
{
   // Slot used to handle "Activated" signal coming from any GL viewer.
   // Used to know which GL viewer is active.

   static Pixel_t green = 0;
   // set the actual GL viewer frame to default color
   if (fActViewer)
      fActViewer->GetFrame()->ChangeBackground(GetDefaultFrameBackground());

   // change the actual GL viewer to the one who emitted the signal
   fActViewer = (TGLEmbeddedViewer *)gTQSender;

   if (fActViewer == 0) return;

   // get the highlight color (only once)
   if (green == 0) {
      fClient->GetColorByName("green", green);
   }
   // set the new actual GL viewer frame to highlight color
   fActViewer->GetFrame()->ChangeBackground(green);

   // update menu entries to match actual viewer's options
   if (fActViewer->GetOrthoXOYCamera()->GetDollyToZoom() &&
       fActViewer->GetOrthoXOZCamera()->GetDollyToZoom() &&
       fActViewer->GetOrthoZOYCamera()->GetDollyToZoom())
      fMenuCamera->UnCheckEntry(kGLOrthoDolly);
   else
      fMenuCamera->CheckEntry(kGLOrthoDolly);

   if (fActViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
       fActViewer->GetOrthoXOYCamera()->GetEnableRotate() &&
       fActViewer->GetOrthoXOYCamera()->GetEnableRotate())
      fMenuCamera->CheckEntry(kGLOrthoRotate);
   else
      fMenuCamera->UnCheckEntry(kGLOrthoRotate);
}

//______________________________________________________________________________
void SplitGLViewFrame::OpenFile(const char *fname)
{
   // Open a Root file to display a geometry in the GL viewers.

   TString filename = fname;
   // check if the file type is correct
   if (!filename.EndsWith(".root")) {
      new TGMsgBox(gClient->GetRoot(), this, "OpenFile",
                   Form("The file \"%s\" is not a root file!", fname),
                   kMBIconExclamation, kMBOk);
      return;
   }
   // check if the root file contains a geometry
   if (TGeoManager::Import(fname) == 0) {
      new TGMsgBox(gClient->GetRoot(), this, "OpenFile",
                   Form("The file \"%s\" does't contain a geometry", fname),
                   kMBIconExclamation, kMBOk);
      return;
   }
   gGeoManager->DefaultColors();
   // delete previous primitives (if any)
   fPad->GetListOfPrimitives()->Delete();
   // and add the geometry to eve pad (container)
   fPad->GetListOfPrimitives()->Add(gGeoManager->GetTopVolume());
   // paint the geometry in each GL viewer
   fTLViewer->PadPaint(fPad);
   fTRViewer->PadPaint(fPad);
   fBLViewer->PadPaint(fPad);
   fBRViewer->PadPaint(fPad);
}

//______________________________________________________________________________
void SplitGLViewFrame::ToggleOrthoRotate()
{
   // Toggle state of the 'Ortho allow rotate' menu entry.

   if (fMenuCamera->IsEntryChecked(kGLOrthoRotate))
      fMenuCamera->UnCheckEntry(kGLOrthoRotate);
   else
      fMenuCamera->CheckEntry(kGLOrthoRotate);
   Bool_t state = fMenuCamera->IsEntryChecked(kGLOrthoRotate);
   if (fActViewer) {
      fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
      fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
      fActViewer->GetOrthoXOYCamera()->SetEnableRotate(state);
   }
}

//______________________________________________________________________________
void SplitGLViewFrame::ToggleOrthoDolly()
{
   // Toggle state of the 'Ortho allow dolly' menu entry.

   if (fMenuCamera->IsEntryChecked(kGLOrthoDolly))
      fMenuCamera->UnCheckEntry(kGLOrthoDolly);
   else
      fMenuCamera->CheckEntry(kGLOrthoDolly);
   Bool_t state = ! fMenuCamera->IsEntryChecked(kGLOrthoDolly);
   if (fActViewer) {
      fActViewer->GetOrthoXOYCamera()->SetDollyToZoom(state);
      fActViewer->GetOrthoXOZCamera()->SetDollyToZoom(state);
      fActViewer->GetOrthoZOYCamera()->SetDollyToZoom(state);
   }
}

// Linkdef
#ifdef __CINT__

#pragma link C++ class SplitGLViewFrame;

#endif
