#include "TPadPainter.h"
#include "TVirtualX.h"

ClassImp(TPadPainter)

//______________________________________________________________________________
TPadPainter::TPadPainter()
{
   //Empty ctor. Here only because of explicit copy ctor.
}

/*
Line/fill/etc. attributes can be set inside TPad, but not only where: 
many of them are set by base sub-objects of 2d primitives
(2d primitives usually inherit TAttLine or TAttFill etc.).  And these sub-objects
call gVirtualX->SetLineWidth ... etc. So, if I save some attributes in my painter,
it will be mess - at any moment I do not know, where to take line attribute - from
gVirtualX or from my own member. So! All attributed, _ALL_ go to/from gVirtualX.
*/

//______________________________________________________________________________
Color_t TPadPainter::GetLineColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineColor();
}

//______________________________________________________________________________
Style_t TPadPainter::GetLineStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineStyle();
}

//______________________________________________________________________________
Width_t TPadPainter::GetLineWidth() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineWidth();
}

//______________________________________________________________________________
void TPadPainter::SetLineColor(Color_t lcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineColor(lcolor);
}

//______________________________________________________________________________
void TPadPainter::SetLineStyle(Style_t lstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineStyle(lstyle);
}

//______________________________________________________________________________
void TPadPainter::SetLineWidth(Width_t lwidth)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineWidth(lwidth);
}

//______________________________________________________________________________
Color_t TPadPainter::GetFillColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillColor();
}

//______________________________________________________________________________
Style_t TPadPainter::GetFillStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillStyle();
}

//______________________________________________________________________________
Bool_t TPadPainter::IsTransparent() const
{
   //Delegate to gVirtualX.
   //IsTransparent is implemented as inline function in TAttFill.
   return gVirtualX->IsTransparent();
}

//______________________________________________________________________________
void TPadPainter::SetFillColor(Color_t fcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillColor(fcolor);
}

//______________________________________________________________________________
void TPadPainter::SetFillStyle(Style_t fstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillStyle(fstyle);
}

//______________________________________________________________________________
void TPadPainter::SetOpacity(Int_t percent)
{
   //Delegate to gVirtualX.
   gVirtualX->SetOpacity(percent);
}

//______________________________________________________________________________
Short_t TPadPainter::GetTextAlign() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAlign();
}

//______________________________________________________________________________
Float_t TPadPainter::GetTextAngle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAngle();
}

//______________________________________________________________________________
Color_t TPadPainter::GetTextColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextColor();
}

//______________________________________________________________________________
Font_t TPadPainter::GetTextFont() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextFont();
}

//______________________________________________________________________________
Float_t TPadPainter::GetTextSize() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextSize();
}

//______________________________________________________________________________
Float_t TPadPainter::GetTextMagnitude() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextMagnitude();
}


//______________________________________________________________________________
void TPadPainter::SetTextAlign(Short_t align)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAlign(align);
}

//______________________________________________________________________________
void TPadPainter::SetTextAngle(Float_t tangle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAngle(tangle);
}

//______________________________________________________________________________
void TPadPainter::SetTextColor(Color_t tcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextColor(tcolor);
}

//______________________________________________________________________________
void TPadPainter::SetTextFont(Font_t tfont)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextFont(tfont);
}

//______________________________________________________________________________
void TPadPainter::SetTextSize(Float_t tsize)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSize(tsize);
}

//______________________________________________________________________________
void TPadPainter::SetTextSizePixels(Int_t npixels)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSizePixels(npixels);
}

/*
"Pixmap" part of TPadPainter.
*/

//______________________________________________________________________________
Int_t TPadPainter::CreateDrawable(UInt_t w, UInt_t h)
{
   return gVirtualX->OpenPixmap(Int_t(w), Int_t(h));
}

//______________________________________________________________________________
void TPadPainter::ClearDrawable()
{
   gVirtualX->Clear();
}

//______________________________________________________________________________
void TPadPainter::CopyDrawable(Int_t id, Int_t px, Int_t py)
{
   gVirtualX->CopyPixmap(id, px, py);
}

//______________________________________________________________________________
void TPadPainter::DestroyDrawable()
{
   gVirtualX->ClosePixmap();
}

//______________________________________________________________________________
void TPadPainter::SelectDrawable(Int_t device)
{
   gVirtualX->SelectWindow(device);
}

//______________________________________________________________________________
void TPadPainter::InitPainter()
{
   //Nothing to do for gVirtualX.
}

/*
Now, the most interesting part of TPadPainter: painting operations.
*/

//______________________________________________________________________________
void TPadPainter::DrawLine(Double_t /*x1*/, Double_t /*y1*/, Double_t /*x2*/, Double_t /*y2*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawLineNDC(Double_t /*u1*/, Double_t /*v1*/, Double_t /*u2*/, Double_t /*v2*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawBox(Double_t /*x1*/, Double_t /*y1*/, Double_t /*x2*/, Double_t /*y2*/, EBoxMode /*mode*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawFillArea(Int_t /*n*/, const Double_t */*x*/, const Double_t */*y*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawFillArea(Int_t /*n*/, const Float_t */*x*/, const Float_t */*y*/)
{
   //
}


//______________________________________________________________________________
void TPadPainter::DrawPolyLine(Int_t /*n*/, const Double_t */*x*/, const Double_t */*y*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawPolyLine(Int_t /*n*/, const Float_t */*x*/, const Float_t */*y*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawPolyLineNDC(Int_t /*n*/, const Double_t */*u*/, const Double_t */*v*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawPolyMarker(Int_t /*n*/, const Double_t */*x*/, const Double_t */*y*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawPolyMarker(Int_t /*n*/, const Float_t */*x*/, const Float_t */*y*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawText(Double_t /*x*/, Double_t /*y*/, Double_t /*angle*/, Double_t /*mgn*/,
                           const char */*text*/, ETextMode /*mode*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::DrawTextNDC(Double_t /*u*/, Double_t /*v*/, Double_t /*angle*/, Double_t /*mgn*/,
                           const char */*text*/, ETextMode /*mode*/)
{
   //
}

//______________________________________________________________________________
void TPadPainter::InvalidateCS()
{
   //
}
