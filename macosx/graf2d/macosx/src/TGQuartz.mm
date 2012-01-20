#include <algorithm>

#include <Cocoa/Cocoa.h>

//DEBUG ONLY
#include <iostream>
//

//#include "Cocoa"

#include "QuartzFonts.h"


#include "TGQuartz.h"
#include "TPoint.h"
#include "TColor.h"
#include "TMath.h"
#include "TStyle.h"
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "RStipples.h"
#include "TROOT.h"

const Double_t kPI = TMath::Pi();

static Int_t gFillHollow;  // Flag if fill style is hollow
static Int_t gFillPattern; // Fill pattern


ClassImp(TGQuartz)


//______________________________________________________________________________
TGQuartz::TGQuartz()
{
   // TGQuartz default constructor
}


//______________________________________________________________________________
TGQuartz::TGQuartz(const char *name, const char *title)
            : TGCocoa(name, title)
{
   // TGQuartz normal constructor
}


//______________________________________________________________________________
void TGQuartz::DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode)
{
   // Draw a box

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   SetContextFillStyle(GetFillStyle());
   
   CGContextSaveGState(ctx);

   if (x1 > x2) std::swap(x1, x2);
   if (y1 > y2) std::swap(y1, y2);

   if (mode == kFilled) {
      if (!gFillPattern) SetContextFillColor(GetFillColor());
      CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   } else {
      SetContextStrokeColor(GetLineColor());
      CGContextStrokeRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   }
   
   CGContextRestoreGState(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawCellArray(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, 
                             Int_t /*nx*/, Int_t /*ny*/, Int_t */*ic*/)
{
   // Draw CellArray
   
   //CGContextRef ctx = (CGContextRef)GetCurrentContext();
}


//______________________________________________________________________________
void TGQuartz::DrawFillArea(Int_t n, TPoint * xy)
{
   // Draw a filled area through all points.
   // n         : number of points
   // xy        : list of points

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   SetContextFillStyle(GetFillStyle());
   
   CGContextBeginPath (ctx);

   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);

   for (Int_t i=1; i<n; i++) CGContextAddLineToPoint (ctx, xy[i].fX  , xy[i].fY);

   if (gFillHollow) {
     SetContextStrokeColor(GetFillColor());
     CGContextStrokePath(ctx);
   } else {
      if (!gFillPattern) SetContextFillColor(GetFillColor());
      CGContextFillPath(ctx);
   }
}


//______________________________________________________________________________
void TGQuartz::DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   // Draw a line.
   // x1,y1        : begin of line
   // x2,y2        : end of line
      
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   SetContextStrokeColor(GetLineColor());
   
   SetContextLineStyle(GetLineStyle());
   
   SetContextLineWidth(GetLineWidth());
   
   CGContextBeginPath (ctx);
   CGContextMoveToPoint(ctx, x1, y1);
   CGContextAddLineToPoint(ctx, x2, y2);
   CGContextStrokePath(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawPolyLine(Int_t n, TPoint *xy)
{
   // Draw a line through all points.
   // n         : number of points
   // xy        : list of points   
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   SetContextStrokeColor(GetLineColor());
   
   SetContextLineStyle(GetLineStyle());
   
   SetContextLineWidth(GetLineWidth());

   CGContextBeginPath (ctx);
   
   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);

   for (Int_t i=1; i<n; i++) CGContextAddLineToPoint (ctx, xy[i].fX  , xy[i].fY);
      
   CGContextStrokePath(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t /*n*/, TPoint * /*xy*/)
{
   // Draw PolyMarker
   
   //CGContextRef ctx = (CGContextRef)GetCurrentContext();
}


//______________________________________________________________________________
void TGQuartz::DrawText(Int_t x, Int_t y, Float_t angle, Float_t /*mgn*/, const char *text, ETextMode /*mode*/)
{
   // Draw text
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
 
   // Text color
   SetContextFillColor(GetTextColor());

  // Text rotation
   CGAffineTransform tm; 
   tm =  CGAffineTransformMakeRotation (-(angle*kPI)/180.);
   tm =  CGAffineTransformScale (tm, 1., -1.); 
   CGContextSetTextMatrix (ctx, tm); 
   
   // Draw the text
   CGContextShowTextAtPoint (ctx, (Float_t)x, (Float_t)y, text, strlen(text)); 
   
/*
 CTFontRef currentFont = fFontManager->SelectFont(gVirtualX->GetTextFont(), gVirtualX->GetTextSize());
   
  //    if (!fSelectedFont)
  //    throw std::runtime_error("GetTextBounds: font not selected");
   
   Font_t fontIndex = gVirtualX->GetTextFont() / 10 - 1;

   UInt_t w = 0, h = 0;
//   if (fontIndex == 11) {
//      CTLineGuard ctLine(text, fSelectedFont, fSymbolMap);
//      ctLine.GetBounds(w, h);
//   } else {
      CTLineGuard ctLine(text, fSelectedFont);
      ctLine.GetBounds(w, h);
//   }
   
   
   CGContextSaveGState(ctx);

   CGContextSetTextPosition(ctx, 0.f, 0.f);
   CGContextTranslateCTM(ctx, x, y);  
   CGContextRotateCTM(ctx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   CGContextTranslateCTM(ctx, xc, yc);
   CGContextTranslateCTM(ctx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, ctx);
   CGContextRestoreGState(ctx);
 */
}

/*
//_________________________________________________________________
void Painter::DrawText(Double_t x, Double_t y, const CTLineGuard &ctLine)
{
   UInt_t w = 0, h = 0;
   ctLine.GetBounds(w, h);
   
   x = fConverter.XToView(x);
   y = fConverter.YToView(y);
   
   Double_t xc = 0., yc = 0.;
   
   const UInt_t hAlign = UInt_t(gVirtualX->GetTextAlign() / 10);   
   switch (hAlign) {
   case 1:
      xc = 0.5 * w;
      break;
   case 2:
      break;
   case 3:
      xc = -0.5 * w;
      break;
   }

   const UInt_t vAlign = UInt_t(gVirtualX->GetTextAlign() % 10);
   switch (vAlign) {
   case 1:
      yc = 0.5 * h;
      break;
   case 2:
      break;
   case 3:
      yc = -0.5 * h;
      break;
   }
   
   CGContextSaveGState(fCtx);

   CGContextSetTextPosition(fCtx, 0.f, 0.f);
   CGContextTranslateCTM(fCtx, x, y);  
   CGContextRotateCTM(fCtx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   CGContextTranslateCTM(fCtx, xc, yc);
   CGContextTranslateCTM(fCtx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, fCtx);
   CGContextRestoreGState(fCtx);
}*/


//______________________________________________________________________________
void TGQuartz::SetContextFillColor(Int_t ci)
{
   // Set the current fill color in the current context.

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   TColor *color = gROOT->GetColor(ci);
   if (!color) return;

   const Float_t a = 1.f;
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;

   color->GetRGB(r, g, b);

   CGContextSetRGBFillColor (ctx, r, g, b, a);
}


//______________________________________________________________________________
void TGQuartz::SetContextStrokeColor(Int_t ci)
{
   // Set the current fill color in the current context.

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   TColor *color = gROOT->GetColor(ci);
   if (!color) return;

   const Float_t a = 1.f;
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;

   color->GetRGB(r, g, b);

   CGContextSetRGBStrokeColor (ctx, r, g, b, a);
}


//______________________________________________________________________________
void TGQuartz::SetLineColor(Color_t cindex)
{
   // Set color index "cindex" for drawing lines.

   TAttLine::SetLineColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetLineStyle(Style_t lstyle)
{
   // Set line style.
   
   TAttLine::SetLineStyle(lstyle);
}


//______________________________________________________________________________
void TGQuartz::SetContextLineStyle(Int_t lstyle)
{
   // Set current line style in the current context.

   static Int_t dashed[2] = {3,3};
   static Int_t dotted[2] = {1,2};
   static Int_t dasheddotted[4] = {3,4,1,4};

   if (lstyle <= 1 ) {
      SetContextLineType(0,0);
   } else if (lstyle == 2 ) {
      SetContextLineType(2,dashed);
   } else if (lstyle == 3 ) {
      SetContextLineType(2,dotted);
   } else if (lstyle == 4 ) {
      SetContextLineType(4,dasheddotted);
   } else {
      TString st = (TString)gStyle->GetLineStyleString(lstyle);
      TObjArray *tokens = st.Tokenize(" ");
      Int_t nt;
      nt = tokens->GetEntries();
      Int_t *linestyle = new Int_t[nt];
      for (Int_t j = 0; j<nt; j++) {
         Int_t it;
         sscanf(((TObjString*)tokens->At(j))->GetName(), "%d", &it);
         linestyle[j] = (Int_t)(it/4);
      }
      SetContextLineType(nt,linestyle);
      delete [] linestyle;
      delete tokens;
   }
}


//______________________________________________________________________________
void TGQuartz::SetContextLineType(Int_t n, Int_t *dash)
{
   // Set the line type in the current context.
   //
   // n       - length of the dash list
   //           n <= 0 use solid lines
   //           n >  0 use dashed lines described by dash(n)
   //                 e.g. n = 4,dash = (6,3,1,3) gives a dashed-dotted line
   //                 with dash length 6 and a gap of 7 between dashes
   // dash(n) - dash segment lengths

   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   if (n) {
      CGFloat lengths[n];
      for (int i=0; i<n;i++) lengths[i] = (CGFloat)dash[i];
      CGContextSetLineDash(ctx,0,lengths,n);
   } else {
      CGContextSetLineDash(ctx,0,NULL,0);
   }
}


//______________________________________________________________________________
void TGQuartz::SetLineWidth(Width_t width)
{
   // Set the line width.
   
   TAttLine::SetLineWidth(width);
}


//______________________________________________________________________________
void TGQuartz::SetContextLineWidth(Int_t width)
{
   // Set the line width in the current context.
   //
   // width - the line width in pixels

   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   if (width<0) return;

   CGFloat w = (CGFloat) fLineWidth;
   CGContextSetLineWidth(ctx, w);
}


//______________________________________________________________________________
void TGQuartz::SetFillColor(Color_t cindex)
{
   // Set color index "cindex" for fill areas.

   TAttFill::SetFillColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetFillStyle(Style_t style)
{
   // Set fill area style.
   
   TAttFill::SetFillStyle(style);
}


//______________________________________________________________________________
void TGQuartz::SetContextFillStyle(Int_t id)
{
   // Set fill area style.
   //
   // style - compound fill area interior style
   //         style = 1000 * interiorstyle + styleindex

   Int_t fais = id/1000;
   Int_t fasi = id%1000;   
   
   switch (fais) {
      case 1:         // solid
         gFillHollow  = 0;
         gFillPattern = 0;
         break;

      case 2:         // pattern
         gFillHollow = 1;
         break;

      case 3:         // hatch
         gFillHollow  = 0;
         gFillPattern = fasi;
         SetStencilPattern();
         break;
         
      default:
         gFillHollow = 1;
         break;
   }
}


//______________________________________________________________________________
static void DrawStencil (void *sti, CGContextRef ctx)
{
   // Draw a stencil pattern from gStipples
   
   int i,j;
   
   int *st = static_cast<int *>(sti);

   int x , y=0; 
   for (i=0; i<31; i=i+2) {
      x = 0;
      for (j=0; j<8; j++) {
         if (gStipples[*st][i] & (1<<j)) CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      for (j=0; j<8; j++) {
         if (gStipples[*st][i+1] & (1<<j)) CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      y++;
   }
}


//______________________________________________________________________________
void TGQuartz::SetStencilPattern()
{
   // Set the fill pattern

   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   CGPatternRef pattern;
   CGColorSpaceRef baseSpace;
   CGColorSpaceRef patternSpace;
   
   TColor *color = gROOT->GetColor(GetFillColor());
   if (!color) return;

   const Float_t a = 1.f;
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;

   color->GetRGB(r, g, b);
   
   CGFloat RGB[4];
   RGB[0] = r;
   RGB[1] = g;
   RGB[2] = b;
   RGB[3] = a;
   CGPatternCallbacks callbacks = {0, &DrawStencil, NULL};
 
   baseSpace = CGColorSpaceCreateDeviceRGB ();
   patternSpace = CGColorSpaceCreatePattern (baseSpace);
   CGContextSetFillColorSpace (ctx, patternSpace);
   CGColorSpaceRelease (patternSpace);
   CGColorSpaceRelease (baseSpace);
   
   pattern = CGPatternCreate(&gFillPattern, CGRectMake(0, 0, 16, 16),
                             CGAffineTransformIdentity, 16, 16,
                             kCGPatternTilingConstantSpacing,
                             false, &callbacks);
   CGContextSetFillPattern (ctx, pattern, RGB);
   CGPatternRelease (pattern);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerColor(Color_t cindex)
{
   // Set color index "cindex" for markers.

   TAttMarker::SetMarkerColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerSize(Float_t markersize)
{
   // Set marker size index.
   //
   // markersize - the marker scale factor

   TAttMarker::SetMarkerSize(markersize);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerStyle(Style_t markerstyle)
{
   // Set marker style.

   TAttMarker::SetMarkerStyle(markerstyle);
}


//______________________________________________________________________________
void TGQuartz::SetTextAlign(Short_t talign)
{
   // Set the text alignment.
   //
   // talign = txalh horizontal text alignment
   // talign = txalv vertical text alignment

   TAttText::SetTextAlign(talign);
}


//______________________________________________________________________________
void TGQuartz::SetTextColor(Color_t cindex)
{
   // Set the color index "cindex" for text.

   TAttText::SetTextColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetTextFont(Font_t fontnumber)
{
   // Set the current text font number.

   TAttText::SetTextFont(fontnumber);
/*   
   static const char *fontname[] = {
      "Times-Italic"         , "Times-Bold"         , "Times-BoldItalic",
      "Helvetica"            , "Helvetica-Oblique"  , "Helvetica-Bold"  ,
      "Helvetica-BoldOblique", "Courier"            , "Courier-Oblique" ,
      "Courier-Bold"         , "Courier-BoldOblique", "Symbol"          ,
      "Times-Roman"          , "ZapfDingbats"       , "Symbol"};
      
   Int_t font = abs(fTextFont)/10;
   if( font > 15 || font < 1) font = 1;
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   if (font<12 || font==13) {
      CGContextSelectFont (ctx, 
                           fontname[font-1],
                           GetTextSize(),
                           kCGEncodingMacRoman);
    } else {
      CGContextSelectFont (ctx, 
                           fontname[font-1],
                           GetTextSize(),
                           kCGEncodingFontSpecific);
    }          
                        
   // Text drawing mode
///CGContextSetTextDrawingMode (ctx, kCGTextFillStroke); 
   CGContextSetTextDrawingMode (ctx, kCGTextFill);  */
}


//______________________________________________________________________________
void TGQuartz::SetTextSize(Float_t textsize)
{
   // Set the current text size to "textsize"
   
   TAttText::SetTextSize(textsize);/*
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   CGContextSetFontSize(ctx, textsize);*/
}


//______________________________________________________________________________
void TGQuartz::SetOpacity(Int_t /*percent*/)
{
   // Set opacity of the current window. This image manipulation routine
   // works by adding to a percent amount of neutral to each pixels RGB.
   // Since it requires quite some additional color map entries is it
   // only supported on displays with more than > 8 color planes (> 256
   // colors).
}


//______________________________________________________________________________
Int_t TGQuartz::SetTextFont(char * /*fontname*/, ETextSetMode /*mode*/)
{
   // Set text font to specified name "fontname".This function returns 0 if
   // the specified font is found, 1 if it is not.
   //
   // mode - loading flag
   //        mode = 0 search if the font exist (kCheck)
   //        mode = 1 search the font and load it if it exists (kLoad)
   
   return 0;
}
