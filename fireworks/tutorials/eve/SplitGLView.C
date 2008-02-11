
#include "TApplication.h"
#include "TSystem.h"
#include "TGFrame.h"
#include "TGLayout.h"
#include "TGSplitter.h"
#include "TGLWidget.h"
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
#include "TEnv.h"
#include "TGListTree.h"
#include "TGTextView.h"

#include "TEveManager.h"
#include "TEveViewer.h"
#include "TEveBrowser.h"
#include "TEveProjectionManager.h"
#include "TEveGeoNode.h"

#include "TGSplitFrame.h"
#include "TGLEmbeddedViewer.h"
#include "TGShapedFrame.h"

#include "TFormula.h"
#include "TF1.h"
#include "TH1F.h"

#ifdef WIN32
#include <TWin32SplashThread.h>
#endif

const char *filetypes[] = { 
   "ROOT files",    "*.root",
   "All files",     "*",
   0,               0 
};

class SplitGLView : public TGMainFrame {

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
   TGLEmbeddedViewer *fViewer0;     // main GL viewer
   TGLEmbeddedViewer *fViewer1;     // first GL viewer
   TGLEmbeddedViewer *fViewer2;     // second GL viewer
   TGLEmbeddedViewer *fActViewer;   // actual (active) GL viewer
   TGTextView        *fTextView;
   TGMenuBar         *fMenuBar;     // main menu bar
   TGPopupMenu       *fMenuFile;    // 'File' popup menu
   TGPopupMenu       *fMenuHelp;    // 'Help' popup menu
   TGPopupMenu       *fMenuCamera;  // 'Camera' popup menu
   TGPopupMenu       *fMenuScene;   // 'Scene' popup menu
   TGStatusBar       *fStatusBar;   // status bar
   TGShapedToolTip   *fShapedToolTip;   // shaped tooltip
   Bool_t             fIsEmbedded;

   TEveViewer        *fViewer[3];
   TEveProjectionManager *fRPhiMgr;
   TEveProjectionManager *fRhoZMgr;

public:
   SplitGLView(const TGWindow *p=0, UInt_t w=800, UInt_t h=600, Bool_t embed=kFALSE);
   virtual ~SplitGLView();

   void           ItemClicked(TGListTreeItem *item, Int_t btn, Int_t x, Int_t y);
   void           HandleMenu(Int_t id);
   void           OnClicked(TObject *obj);
   void           OnDoubleClick();
   void           OnMouseIdle(TGLPhysicalShape *shape, UInt_t posx, UInt_t posy);
   void           OnMouseOver(TGLPhysicalShape *shape);
   void           OnViewerActivated();
   void           OpenFile(const char *fname);
   void           ToggleOrthoRotate();
   void           ToggleOrthoDolly();

   TEveProjectionManager *GetRPhiMgr() const { return fRPhiMgr; }
   TEveProjectionManager *GetRhoZMgr() const { return fRhoZMgr; }

   ClassDef(SplitGLView, 0)
};

TEveProjectionManager *gRPhiMgr = 0;
TEveProjectionManager *gRhoZMgr = 0;

ClassImp(SplitGLView)

//______________________________________________________________________________
SplitGLView::SplitGLView(const TGWindow *p, UInt_t w, UInt_t h, Bool_t embed) :
   TGMainFrame(p, w, h), fActViewer(0), fShapedToolTip(0), fIsEmbedded(embed)
{
   // Main frame constructor.

   TGSplitFrame *frm;
   TEveScene *s;

   // create the "file" popup menu
   fMenuFile = new TGPopupMenu(gClient->GetRoot());
   fMenuFile->AddEntry("&Open...", kFileOpen);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", kFileExit);

   // create the "camera" popup menu
   fMenuCamera = new TGPopupMenu(gClient->GetRoot());
   fMenuCamera->AddEntry("Perspective (Floor XOZ)", kGLPerspXOZ);
   fMenuCamera->AddEntry("Perspective (Floor YOZ)", kGLPerspYOZ);
   fMenuCamera->AddEntry("Perspective (Floor XOY)", kGLPerspXOY);
   fMenuCamera->AddEntry("Orthographic (XOY)", kGLXOY);
   fMenuCamera->AddEntry("Orthographic (XOZ)", kGLXOZ);
   fMenuCamera->AddEntry("Orthographic (ZOY)", kGLZOY);
   fMenuCamera->AddSeparator();
   fMenuCamera->AddEntry("Ortho allow rotate", kGLOrthoRotate);
   fMenuCamera->AddEntry("Ortho allow dolly",  kGLOrthoDolly);

   fMenuScene = new TGPopupMenu(gClient->GetRoot());
   fMenuScene->AddEntry("&Update Current", kSceneUpdate);
   fMenuScene->AddEntry("Update &All", kSceneUpdateAll);

   // create the "help" popup menu
   fMenuHelp = new TGPopupMenu(gClient->GetRoot());
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
   fMenuFile->Connect("Activated(Int_t)", "SplitGLView", this,
                      "HandleMenu(Int_t)");
   fMenuCamera->Connect("Activated(Int_t)", "SplitGLView", this,
                        "HandleMenu(Int_t)");
   fMenuScene->Connect("Activated(Int_t)", "SplitGLView", this,
                       "HandleMenu(Int_t)");
   fMenuHelp->Connect("Activated(Int_t)", "SplitGLView", this,
                      "HandleMenu(Int_t)");
   
   if (fIsEmbedded && gEve) {
      // use status bar from the browser
      fStatusBar = gEve->GetBrowser()->GetStatusBar();
   }
   else {
      // create the status bar
      Int_t parts[] = {45, 15, 10, 30};
      fStatusBar = new TGStatusBar(this, 50, 10);
      fStatusBar->SetParts(parts, 4);
      AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 
               0, 0, 10, 0));
   }

   // create eve pad (our geometry container)
   fPad = new TEvePad();

   // create the split frames
   fSplitFrame = new TGSplitFrame(this, 800, 600);
   AddFrame(fSplitFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   // split it once
   fSplitFrame->HSplit(434);
   // then split each part again (this will make four parts)
   fSplitFrame->GetSecond()->VSplit(266);
   fSplitFrame->GetSecond()->GetSecond()->VSplit(266);

   // get top (main) split frame
   frm = fSplitFrame->GetFirst();
   // create (embed) a GL viewer inside
   fViewer0 = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fViewer0->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to perspective (XOZ) for this viewer
   fViewer0->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
   // connect signal we are interested to
   fViewer0->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLView", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fViewer0->Connect("Activated()", "SplitGLView", this, 
                      "OnViewerActivated()");
   fViewer0->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLView", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   fViewer0->Connect("DoubleClicked()", "SplitGLView", this, 
                      "OnDoubleClick()");
   fViewer0->Connect("Clicked(TObject*)", "SplitGLView", this, 
                      "OnClicked(TObject*)");
   fViewer[0] = new TEveViewer("SplitGLViewer[0]");
   fViewer[0]->SetGLViewer(fViewer0);
   fViewer[0]->IncDenyDestroy();
   if (fIsEmbedded && gEve) {
      fViewer[0]->AddScene(gEve->GetGlobalScene());
      fViewer[0]->AddScene(gEve->GetEventScene());
      gEve->AddElement(fViewer[0], gEve->GetViewers());
   }

   s = gEve->SpawnNewScene("Rho-Z Projection");
   // projections
   fRhoZMgr = new TEveProjectionManager();
   gEve->AddElement(fRhoZMgr, (TEveElement *)s);
   gEve->AddToListTree(fRhoZMgr, kTRUE);

   // get bottom left split frame
   frm = fSplitFrame->GetSecond()->GetFirst();
   // create (embed) a GL viewer inside
   fViewer1 = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fViewer1->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to orthographic (XOY) for this viewer
   fViewer1->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   // connect signal we are interested to
   fViewer1->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLView", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fViewer1->Connect("Activated()", "SplitGLView", this, 
                      "OnViewerActivated()");
   fViewer1->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLView", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   fViewer1->Connect("DoubleClicked()", "SplitGLView", this, 
                     "OnDoubleClick()");
   fViewer1->Connect("Clicked(TObject*)", "SplitGLView", this, 
                      "OnClicked(TObject*)");
   fViewer[1] = new TEveViewer("SplitGLViewer[1]");
   fViewer[1]->SetGLViewer(fViewer1);
   fViewer[1]->IncDenyDestroy();
   if (fIsEmbedded && gEve) {
      fRhoZMgr->ImportElements((TEveElement *)gEve->GetGlobalScene());
      fRhoZMgr->ImportElements((TEveElement *)gEve->GetEventScene());
      fRhoZMgr->SetProjection(TEveProjection::kPT_RhoZ);
      fViewer[1]->AddScene(s);
      gEve->AddElement(fViewer[1], gEve->GetViewers());
   }
   gRhoZMgr = fRhoZMgr;

   s = gEve->SpawnNewScene("R-Phi Projection");
   // projections
   fRPhiMgr = new TEveProjectionManager();
   gEve->AddElement(fRPhiMgr, (TEveElement *)s);
   gEve->AddToListTree(fRPhiMgr, kTRUE);

   // get bottom center split frame
   frm = fSplitFrame->GetSecond()->GetSecond()->GetFirst();
   // create (embed) a GL viewer inside
   fViewer2 = new TGLEmbeddedViewer(frm, fPad);
   frm->AddFrame(fViewer2->GetFrame(), new TGLayoutHints(kLHintsExpandX | 
                 kLHintsExpandY));
   // set the camera to orthographic (XOY) for this viewer
   fViewer2->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   // connect signal we are interested to
   fViewer2->Connect("MouseOver(TGLPhysicalShape*)", "SplitGLView", this, 
                      "OnMouseOver(TGLPhysicalShape*)");
   fViewer2->Connect("Activated()", "SplitGLView", this, 
                      "OnViewerActivated()");
   fViewer2->Connect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                      "SplitGLView", this, 
                      "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   fViewer2->Connect("DoubleClicked()", "SplitGLView", this, 
                     "OnDoubleClick()");
   fViewer2->Connect("Clicked(TObject*)", "SplitGLView", this, 
                      "OnClicked(TObject*)");
   fViewer[2] = new TEveViewer("SplitGLViewer[2]");
   fViewer[2]->SetGLViewer(fViewer2);
   fViewer[2]->IncDenyDestroy();
   if (fIsEmbedded && gEve) {
      fRPhiMgr->ImportElements((TEveElement *)gEve->GetGlobalScene());
      fRPhiMgr->ImportElements((TEveElement *)gEve->GetEventScene());
      fRPhiMgr->SetProjection(TEveProjection::kPT_RPhi);
      fViewer[2]->AddScene(s);
      gEve->AddElement(fViewer[2], gEve->GetViewers());
   }
   gRPhiMgr = fRPhiMgr;

   // get bottom right split frame
   frm = fSplitFrame->GetSecond()->GetSecond()->GetSecond();
   // create (embed) a text view inside
   fTextView = new TGTextView(frm, 100, 100, kSunkenFrame | kDoubleBorder);
   frm->AddFrame(fTextView, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   if (fIsEmbedded && gEve) {
      gEve->GetListTree()->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
         "SplitGLView", this, "ItemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
   }

   fShapedToolTip = new TGShapedToolTip("Default.png", 120, 22, 160, 110, 
                                        23, 115, 12, "#ffff80");
   Resize(GetDefaultSize());
   MapSubwindows();
   MapWindow();
}

//______________________________________________________________________________
SplitGLView::~SplitGLView()
{
   // Clean up main frame...
   //Cleanup();
   
   fMenuFile->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fMenuCamera->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fMenuScene->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fMenuHelp->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fViewer0->Disconnect("MouseOver(TGLPhysicalShape*)", this, 
                         "OnMouseOver(TGLPhysicalShape*)");
   fViewer0->Disconnect("Activated()", this, "OnViewerActivated()");
   fViewer0->Disconnect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                         this, "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   fViewer1->Disconnect("MouseOver(TGLPhysicalShape*)", this, 
                         "OnMouseOver(TGLPhysicalShape*)");
   fViewer1->Disconnect("Activated()", this, "OnViewerActivated()");
   fViewer1->Disconnect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                         this, "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   fViewer2->Disconnect("MouseOver(TGLPhysicalShape*)", this, 
                         "OnMouseOver(TGLPhysicalShape*)");
   fViewer2->Disconnect("Activated()", this, "OnViewerActivated()");
   fViewer2->Disconnect("MouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)", 
                         this, "OnMouseIdle(TGLPhysicalShape*,UInt_t,UInt_t)");
   if (fIsEmbedded && gEve) {
      gEve->GetListTree()->Disconnect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
               this, "ItemClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
      gEve->RemoveElement(fViewer[0], gEve->GetViewers());
      gEve->RemoveElement(fViewer[1], gEve->GetViewers());
      gEve->RemoveElement(fViewer[2], gEve->GetViewers());
   }
   delete fViewer[0];
   delete fViewer[1];
   delete fViewer[2];
   delete fShapedToolTip;
   delete fMenuFile;
   delete fMenuScene;
   delete fMenuCamera;
   delete fMenuHelp;
   delete fMenuBar;
   delete fViewer0;
   delete fViewer1;
   delete fViewer2;
   delete fSplitFrame;
   delete fPad;
   if (!fIsEmbedded) {
      delete fStatusBar;
      gApplication->Terminate(0);
   }
}

//______________________________________________________________________________
void SplitGLView::HandleMenu(Int_t id)
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
         fViewer0->UpdateScene();
         fViewer1->UpdateScene();
         fViewer2->UpdateScene();
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
void SplitGLView::OnMouseIdle(TGLPhysicalShape *shape, UInt_t posx, UInt_t posy)
{
   // Slot used to handle "OnMouseIdle" signal coming from any GL viewer.
   // We receive a pointer on the physical shape in which the mouse cursor is
   // and the actual cursor position (x,y)

   Window_t wtarget;
   Int_t    x = 0, y = 0;

   static TH1F *h1f = 0;
   TFormula *form1 = new TFormula("form1","abs(sin(x)/x)");
   form1->Update(); // silent warning about unused variable...
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
               gClient->GetDefaultRoot()->GetId(), posx, posy, x, y,
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
void SplitGLView::OnMouseOver(TGLPhysicalShape *shape)
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
void SplitGLView::OnViewerActivated()
{
   // Slot used to handle "Activated" signal coming from any GL viewer.
   // Used to know which GL viewer is active.

   static Pixel_t green = 0;

   // set the actual GL viewer frame to default color
   if (fActViewer)
      fActViewer->GetFrame()->ChangeBackground(GetDefaultFrameBackground());

   // change the actual GL viewer to the one who emitted the signal
   // fActViewer = (TGLEmbeddedViewer *)gTQSender;
   fActViewer = dynamic_cast<TGLEmbeddedViewer*>((TQObject*)gTQSender);

   if (fActViewer == 0) {
      printf ("dyncast failed ...\n");
      return;
   }

   // get the highlight color (only once)
   if (green == 0) {
      gClient->GetColorByName("green", green);
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
void SplitGLView::OpenFile(const char *fname)
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
   fViewer0->PadPaint(fPad);
   fViewer1->PadPaint(fPad);
   fViewer2->PadPaint(fPad);
}

//______________________________________________________________________________
void SplitGLView::ToggleOrthoRotate()
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
void SplitGLView::ToggleOrthoDolly()
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

//______________________________________________________________________________
void SplitGLView::ItemClicked(TGListTreeItem *item, Int_t, Int_t, Int_t)
{
   // Item has been clicked, based on mouse button do:

   static const TEveException eh("SplitGLView::ItemClicked ");
   TEveElement* re = (TEveElement*)item->GetUserData();
   if(re == 0) return;
   TObject* obj = re->GetObject(eh);
   if (obj->InheritsFrom("TEveViewer")) {
      TGLViewer *v = ((TEveViewer *)obj)->GetGLViewer();
      //v->Activated();
      if (v->InheritsFrom("TGLEmbeddedViewer")) {
         TGLEmbeddedViewer *ev = (TGLEmbeddedViewer *)v;
         gVirtualX->SetInputFocus(ev->GetGLWindow()->GetContainer()->GetId());
      }
   }
}

//______________________________________________________________________________
void SplitGLView::OnClicked(TObject *obj)
{
   // Handle click events in GL viewer

   if (obj)
      fStatusBar->SetText(Form("User clicked on: \"%s\"", obj->GetName()), 1);
   else
      fStatusBar->SetText("", 1);
}

//______________________________________________________________________________
void SplitGLView::OnDoubleClick()
{
   // Handle double-clicks in GL viewer

   // get the origin (sender) of the signal
   TGLEmbeddedViewer *sourceview = dynamic_cast<TGLEmbeddedViewer*>((TQObject*)gTQSender);
   if (sourceview == 0) return;

   // target: top (main) GL View frame
   TGSplitFrame *dest = fSplitFrame->GetFirst();
   // source frame
   TGCompositeFrame *source = (TGCompositeFrame *)sourceview->GetFrame();
   // get its parent frame (its container)
   TGCompositeFrame *sourceframe = (TGCompositeFrame *)source->GetParent();
   // skip if source and target are the same
   if (sourceframe == dest) return;
   // get the pointer to the frame that has to be exchanged with the 
   // source one (the one actually in the destination)
   TGCompositeFrame *prev = (TGCompositeFrame *)dest->GetFrame();
   // finally swith the frames
   TGSplitFrame::SwitchFrames(sourceview->GetFrame(), dest, prev);
}

// Linkdef
#ifdef __CINT__

#pragma link C++ class SplitGLView;

#endif
