#include <algorithm>

#include <Cocoa/Cocoa.h>

//DEBUG ONLY
#include <iostream>
//

//#include "Cocoa"

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

ClassImp(TGQuartz)


//______________________________________________________________________________
TGQuartz::TGQuartz()
{
   NSLog(@"TGQuartz default ctor");
}


//______________________________________________________________________________
TGQuartz::TGQuartz(const char *name, const char *title)
            : TGCocoa(name, title)
{
   NSLog(@"TGQuartz was created %s %s", name, title);
}


//______________________________________________________________________________
void TGQuartz::DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode)
{
   //This is just a sketch implementation.
   
   //TODO: check fill style (it can be polygon with some stipple).
   //Think about gradient filling and transparency.
   //What about outline? (TODO).
   //What about different color spaces? (using RGB now, but ...)

   CGContextRef ctx = (CGContextRef)fCtx;//This is context from CoreGraphics, it's up to me (TGCocoa) to manage it.

   //This function modifies different state variables in context.
   //The same context will be used by other functions.
   //So either save and restore state (that's what I do here) or
   //REMEMBER to set these states later to correct values 
   //(correct from next operation's POV). 
   CGContextSaveGState(ctx);

   if (mode == kFilled) {
      //Fill the rectangle with solid color.
   
      const Float_t alpha = 1.f; //Alpha for filled area must be set externally, for example,
                                 //add transparency into TAttFill, add global fill transparency
                                 //and check them here.
      Float_t red = 1.f, green = 1.f, blue = 1.f; //White by default.
      //Color in gVirtualX is already set by primitive or by pad,
      //just extract RGB triplet here.
      if (const TColor *color = gROOT->GetColor(GetFillColor()))
         color->GetRGB(red, green, blue); 

      CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
      CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   } else {
      //Find better way to specify line attributes like line caps
      //and joins - TODO: this must go somehow to TAttLine, so we can set/get
      //these parameters externally/internally. In this example - just hardcoded values.
      CGContextSetLineCap(ctx, kCGLineCapRound);
      CGContextSetLineJoin(ctx, kCGLineJoinMiter);
   
      if (GetLineWidth() > 1.f)
         CGContextSetLineWidth(ctx, GetLineWidth());

      const Float_t alpha = 1.f; //Alpha for line must be set externally, for example,
                                 //add transparency into TAttLine, add global line transparency
                                 //and check them here.

      Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
      if (const TColor *color = gROOT->GetColor(GetLineColor()))
         color->GetRGB(red, green, blue);

      CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);

      if (x1 > x2) std::swap(x1, x2);
      if (y1 > y2) std::swap(y1, y2);

      CGContextStrokeRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   }
   
   CGContextRestoreGState(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawCellArray(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, Int_t /*nx*/, Int_t /*ny*/, Int_t */*ic*/)
{
//   CGContextRef ctx = (CGContextRef)fCtx;
}


//______________________________________________________________________________
void TGQuartz::DrawFillArea(Int_t n, TPoint * xy)
{
   // Draw a filled area through all points.
   // n         : number of points
   // xy        : list of points

   CGContextRef ctx = (CGContextRef)fCtx;

 //  SetFillColorIndex(GetFillColor());

   CGContextBeginPath (ctx);

   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);

   for (Int_t i=1; i<n; i++) CGContextAddLineToPoint (ctx, xy[i].fX  , xy[i].fY);

   CGContextFillPath(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   // Draw a line.
   // x1,y1        : begin of line
   // x2,y2        : end of line
      
   CGContextRef ctx = (CGContextRef)fCtx;
   
   SetStrokeColorIndex(GetLineColor());
   
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
   
   CGContextRef ctx = (CGContextRef)fCtx;
   
   SetStrokeColorIndex(GetLineColor());

   CGContextBeginPath (ctx);
   
   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);

   for (Int_t i=1; i<n; i++) CGContextAddLineToPoint (ctx, xy[i].fX  , xy[i].fY);
      
   CGContextStrokePath(ctx);
}


//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t /*n*/, TPoint * /*xy*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}


//______________________________________________________________________________
void TGQuartz::DrawText(Int_t x, Int_t y, Float_t angle, Float_t /*mgn*/, const char *text, ETextMode /*mode*/)
{
   // Draw text
   
   CGContextRef ctx = (CGContextRef)fCtx;
 
   // Text color
   SetFillColorIndex(GetTextColor());

  // Text rotation
   CGAffineTransform tm; 
   tm =  CGAffineTransformMakeRotation (-(angle*kPI)/180.);
   tm =  CGAffineTransformScale (tm, 1., -1.); 
   CGContextSetTextMatrix (ctx, tm); 
   
   // Draw the text
   CGContextShowTextAtPoint (ctx, (Float_t)x, (Float_t)y, text, strlen(text)); 
}


//______________________________________________________________________________
void TGQuartz::SetFillColorIndex(Int_t ci)
{
   // Set the current fill color index.

   CGContextRef ctx = (CGContextRef)fCtx;

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
void TGQuartz::SetStrokeColorIndex(Int_t ci)
{
   // Set the current fill color index.

   CGContextRef ctx = (CGContextRef)fCtx;

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
   // Sets color index "cindex" for drawing lines.

   TAttLine::SetLineColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetLineStyle(Style_t lstyle)
{
   // Set line style.

   static Int_t dashed[2] = {3,3};
   static Int_t dotted[2] = {1,2};
   static Int_t dasheddotted[4] = {3,4,1,4};

   if (fLineStyle != lstyle) { //set style index only if different
      fLineStyle = lstyle;
      if (lstyle <= 1 ) {
         SetLineType(0,0);
      } else if (lstyle == 2 ) {
         SetLineType(2,dashed);
      } else if (lstyle == 3 ) {
         SetLineType(2,dotted);
      } else if (lstyle == 4 ) {
         SetLineType(4,dasheddotted);
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
         SetLineType(nt,linestyle);
         delete [] linestyle;
         delete tokens;
      }
   }
}


//______________________________________________________________________________
void TGQuartz::SetLineType(Int_t n, Int_t *dash)
{
   // Sets the line type.
   //
   // n       - length of the dash list
   //           n <= 0 use solid lines
   //           n >  0 use dashed lines described by dash(n)
   //                 e.g. n = 4,dash = (6,3,1,3) gives a dashed-dotted line
   //                 with dash length 6 and a gap of 7 between dashes
   // dash(n) - dash segment lengths
   
   CGContextRef ctx = (CGContextRef)fCtx;
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
   // Sets the line width.
   //
   // width - the line width in pixels
   
   CGContextRef ctx = (CGContextRef)fCtx;
   if (fLineWidth == width || width<0) return;
   fLineWidth = width;
   CGFloat w = (CGFloat) fLineWidth;
   CGContextSetLineWidth(ctx, w);
}


//______________________________________________________________________________
void TGQuartz::SetFillColor(Color_t cindex)
{
   // Sets color index "cindex" for fill areas.
   TAttFill::SetFillColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetFillStyle(Style_t style)
{
   // Sets fill area style.
   //
   // style - compound fill area interior style
   //         style = 1000 * interiorstyle + styleindex
   
   TAttFill::SetFillStyle(style);   

   if (style == 1234) SetStencilPattern();
}


//______________________________________________________________________________
static void DrawStencil (void */*st*/, CGContextRef ctx)
{
   // Draw a stencil pattern from gStipples
   
   int i,j;

   Int_t st = 25, x , y=0; 
   for (i=0; i<31; i=i+2) {
      x = 0;
      for (j=0; j<8; j++) {
         if (gStipples[st][i] & (1<<j)) CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      for (j=0; j<8; j++) {
         if (gStipples[st][i+1] & (1<<j)) CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      y++;
   }
}


//______________________________________________________________________________
void TGQuartz::SetStencilPattern()
{
   // Set the fill pattern
   
   CGContextRef ctx = (CGContextRef)fCtx;
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
   pattern = CGPatternCreate(NULL, CGRectMake(0, 0, 16, 16),
                             CGAffineTransformIdentity, 16, 16,
                             kCGPatternTilingConstantSpacing,
                             false, &callbacks);
   CGContextSetFillPattern (ctx, pattern, RGB);
   CGPatternRelease (pattern);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerColor(Color_t cindex)
{
   // Sets color index "cindex" for markers.
   TAttMarker::SetMarkerColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerSize(Float_t markersize)
{
   // Sets marker size index.
   //
   // markersize - the marker scale factor
   TAttMarker::SetMarkerSize(markersize);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerStyle(Style_t markerstyle)
{
   // Sets marker style.
   TAttMarker::SetMarkerStyle(markerstyle);
}


//______________________________________________________________________________
void TGQuartz::SetTextAlign(Short_t talign)
{
   // Sets the text alignment.
   //
   // talign = txalh horizontal text alignment
   // talign = txalv vertical text alignment
   TAttText::SetTextAlign(talign);
}


//______________________________________________________________________________
void TGQuartz::SetTextColor(Color_t cindex)
{
   // Sets the color index "cindex" for text.
   TAttText::SetTextColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetTextFont(Font_t fontnumber)
{
   // Sets the current text font number.
   
   TAttText::SetTextFont(fontnumber);
   
   static const char *fontname[] = {
      "Times-Italic"         , "Times-Bold"         , "Times-BoldItalic",
      "Helvetica"            , "Helvetica-Oblique"  , "Helvetica-Bold"  ,
      "Helvetica-BoldOblique", "Courier"            , "Courier-Oblique" ,
      "Courier-Bold"         , "Courier-BoldOblique", "Symbol"          ,
      "Times-Roman"          , "ZapfDingbats"       , "Symbol"};
      
   Int_t font = abs(fTextFont)/10;
   if( font > 15 || font < 1) font = 1;
   
   CGContextRef ctx = (CGContextRef)fCtx;
   
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
   CGContextSetTextDrawingMode (ctx, kCGTextFill);  
}


//______________________________________________________________________________
void TGQuartz::SetTextSize(Float_t textsize)
{
   // Sets the current text size to "textsize"
   
   TAttText::SetTextSize(textsize);
   CGContextRef ctx = (CGContextRef)fCtx;
   CGContextSetFontSize(ctx, textsize);
}


//______________________________________________________________________________
void TGQuartz::SetOpacity(Int_t /*percent*/)
{
   // Sets opacity of the current window. This image manipulation routine
   // works by adding to a percent amount of neutral to each pixels RGB.
   // Since it requires quite some additional color map entries is it
   // only supported on displays with more than > 8 color planes (> 256
   // colors).
}


//______________________________________________________________________________
Int_t TGQuartz::SetTextFont(char * /*fontname*/, ETextSetMode /*mode*/)
{
   // Sets text font to specified name "fontname".This function returns 0 if
   // the specified font is found, 1 if it is not.
   //
   // mode - loading flag
   //        mode = 0 search if the font exist (kCheck)
   //        mode = 1 search the font and load it if it exists (kLoad)
   
   return 0;
}
