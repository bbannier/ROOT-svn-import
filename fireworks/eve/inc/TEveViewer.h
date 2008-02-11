// @(#)root/eve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveViewer
#define ROOT_TEveViewer

#include "TEveElement.h"

class TGWindow;
class TGedEditor;
class TGLViewer;
class TGLPhysicalShape;

class TEveScene;

/******************************************************************************/
// TEveViewer
/******************************************************************************/

class TEveViewer : public TEveElementList
{
private:
   TEveViewer(const TEveViewer&);            // Not implemented
   TEveViewer& operator=(const TEveViewer&); // Not implemented

protected:
   TGLViewer *fGLViewer;

public:
   TEveViewer(const Text_t* n="TEveViewer", const Text_t* t="");
   virtual ~TEveViewer() {}

   TGLViewer* GetGLViewer() const { return fGLViewer; }
   void SetGLViewer(TGLViewer* s);
   void SpawnGLViewer(const TGWindow* parent, TGedEditor* ged);

   virtual void AddScene(TEveScene* scene);

   virtual void RemoveElementLocal(TEveElement* el);
   virtual void RemoveElementsLocal();

   virtual TObject* GetEditorObject(const TEveException& eh="TEveViewer::GetEditorObject ") const;

   virtual Bool_t HandleElementPaste(TEveElement* el);

   virtual const TGPicture* GetListTreeIcon();

   ClassDef(TEveViewer, 0); // Reve representation of TGLViewer.
};


/******************************************************************************/
// TEveViewerList
/******************************************************************************/

class TEveViewerList : public TEveElementList
{
public:
   enum EPickToSelect // How to convert GL picking events to selection:
   {
      kPSIgnore,      // ignore GL picking
      kPSElement,     // select element
      kPSProjectable, // select projectable and all its projections (default)
      kPSCompound     // select compound and all its projections
   };

private:
   TEveViewerList(const TEveViewerList&);            // Not implemented
   TEveViewerList& operator=(const TEveViewerList&); // Not implemented

protected:
   Int_t        fPickToSelect;

public:
   TEveViewerList(const Text_t* n="TEveViewerList", const Text_t* t="");
   virtual ~TEveViewerList() {}

   virtual void Connect();

   void RepaintChangedViewers(Bool_t resetCameras, Bool_t dropLogicals);
   void RepaintAllViewers(Bool_t resetCameras, Bool_t dropLogicals);

   void SceneDestructing(TEveScene* scene);

   // --------------------------------

   Int_t GetPickToSelect()   const { return fPickToSelect; }
   void  SetPickToSelect(Int_t ps) { fPickToSelect = ps; }

   TEveElement* MapPickedToSelected(TEveElement* el);
   void OnMouseOver(TGLPhysicalShape* shape);
   void OnClicked(TObject *obj);

   ClassDef(TEveViewerList, 0); // List of Viewers providing common operations on TEveViewer collections.
};

#endif
