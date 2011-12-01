#ifndef ROOT_TGQuartz
#define ROOT_TGQuartz

#ifndef ROOT_TGCocoa
#include "TGCocoa.h"
#endif

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This is non-GUI part of TVirtualX interface, implemented for         //
// MacOS X, using CoreGraphics (Quartz).                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class TGQuartz : public TGCocoa {
public:
   TGQuartz();
   TGQuartz(const char *name, const char *title);
   
   //Final-overriders for TVirtualX.
   virtual void      DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode);
   virtual void      DrawCellArray(Int_t x1, Int_t y1, Int_t x2, Int_t y2,
                                   Int_t nx, Int_t ny, Int_t *ic);
   virtual void      DrawFillArea(Int_t n, TPoint *xy);
   
   using TGCocoa::DrawLine;
   
   virtual void      DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2);
   virtual void      DrawPolyLine(Int_t n, TPoint *xy);
   virtual void      DrawPolyMarker(Int_t n, TPoint *xy);
   virtual void      DrawText(Int_t x, Int_t y, Float_t angle, Float_t mgn, const char *text,
                              ETextMode mode);
                              
   //This function is just for test,
   //will be removed in future.
   void SetContext(void *ctx);

private:
   TGQuartz(const TGQuartz &rhs);
   TGQuartz &operator = (const TGQuartz &rhs);
   
   void *fCtx;
   
   ClassDef(TGQuartz, 0);//2D graphics for Mac OSX.
};

#endif
