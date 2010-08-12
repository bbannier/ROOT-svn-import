// @(#)root/gl:$Id$
// Author:  Timur Pocheptsov, Jun 2007

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGLWidget
#define ROOT_TGLWidget

#include <utility>
#include <memory>
#include <set>

#ifndef ROOT_TGLContext
#include "TGLContext.h"
#endif
#ifndef ROOT_TVirtualGL
#include "TVirtualGL.h"
#endif
#ifndef ROOT_TGLFormat
#include "TGLFormat.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

class TGLWidget;
class TGEventHandler;

//Base class for window based gl devices.
//Simple adds two new functions to widget's interface.
//Exists to make TGLContext more "generic".

class TGLWidgetBase : public TGLPaintDevice {
protected:
   TGLContext               *fGLContext;
   //fInnerData is for X11 - <dpy, visualInfo> pair.
   std::pair<void *, void *> fInnerData;
   Int_t                     fWindowIndex;

   TGLFormat                 fGLFormat;
   //fFromCtor checks that SetFormat was called only from ctor.
   Bool_t                    fFromInit;

   Handle_t                  fWidgetID;

   std::set<TGLContext *>    fValidContexts;

   TGLWidgetBase();
   ~TGLWidgetBase();
public:

   virtual std::pair<void *, void *> GetInnerData() const {return fInnerData;}
   virtual Handle_t GetWidgetID() const {return fWidgetID;}

   //Overriders.
   Bool_t HasFBO()const;
   Int_t  GetWindowIndex() const {return fWindowIndex;}

   void AddContext(TGLContext *ctx);
   void RemoveContext(TGLContext *ctx);

   //This function is public _ONLY_ for calls
   //via gInterpreter. Do not call it directly.
   void SetFormat();
   //
   Bool_t            MakeCurrent();
   Bool_t            ClearCurrent();
   void              SwapBuffers();
   const TGLContext *GetContext()const;
   const  TGLFormat *GetPixelFormat()const;

private:

   TGLWidgetBase(const TGLWidgetBase &rhs);
   TGLWidgetBase &operator = (const TGLWidgetBase &rhs);

   ClassDef(TGLWidgetBase, 0)//Base class for gl-widgets.
};

class TGLWidget : public TGFrame, public TGLWidgetBase {
   friend class TGLContext;

private:
   TGEventHandler                   *fEventHandler;

public:
   static TGLWidget* Create(const TGWindow* parent, Bool_t selectInput,
             Bool_t shareDefault, const TGLPaintDevice *shareDevice,
             UInt_t width, UInt_t height);

   static TGLWidget* Create(const TGLFormat &format,
             const TGWindow* parent, Bool_t selectInput,
             Bool_t shareDefault, const TGLPaintDevice *shareDevice,
             UInt_t width, UInt_t height);

   ~TGLWidget();

   virtual void      InitGL();
   virtual void      PaintGL();

   //To repaint gl-widget without GUI events.
   void              ExtractViewport(Int_t *vp)const;

   TGEventHandler   *GetEventHandler() const { return fEventHandler; }
   void              SetEventHandler(TGEventHandler *eh);

   Bool_t HandleButton(Event_t *ev);
   Bool_t HandleDoubleClick(Event_t *ev);
   Bool_t HandleConfigureNotify(Event_t *ev);
   Bool_t HandleKey(Event_t *ev);
   Bool_t HandleMotion(Event_t *ev);
   Bool_t HandleFocusChange(Event_t *);
   Bool_t HandleCrossing(Event_t *);

   void   DoRedraw();

protected:
   TGLWidget(Window_t glw, const TGWindow* parent, Bool_t selectInput);



   Handle_t                  GetWidgetID() const {return GetId();}

private:
   TGLWidget(const TGLWidget &);              // Not implemented.
   TGLWidget &operator = (const TGLWidget &); // Not implemented.

   ClassDef(TGLWidget, 0); //Window (widget) version of TGLPaintDevice
};

class TGViewPort;

class TGLCanvasWidget : public TGLWidgetBase {
private:
public:
   TGLCanvasWidget(const TGViewPort *parent);

   void ExtractViewport(Int_t *vp)const;

private:

   TGLCanvasWidget(const TGLCanvasWidget &rhs);
   TGLCanvasWidget & operator = (const TGLCanvasWidget &);

   ClassDef(TGLCanvasWidget, 0)
};

#endif
