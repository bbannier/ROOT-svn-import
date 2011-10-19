#include "TGIOS.h"


namespace ROOT {
namespace iOS {

//______________________________________________________________________________
TGIOS::TGIOS()
{
   // Default ctor.
}

//______________________________________________________________________________
TGIOS::TGIOS(const char *name, const char *title)
         : TVirtualX(name, title)
{
   // Ctor.
}

//TAttLine.
//______________________________________________________________________________
void TGIOS::SetLineColor(Color_t cindex)
{
   // Sets color index "cindex" for drawing lines.
   TAttLine::SetLineColor(cindex);
}

//______________________________________________________________________________
void TGIOS::SetLineStyle(Style_t linestyle)
{
   // Sets the line style.
   //
   // linestyle <= 1 solid
   // linestyle  = 2 dashed
   // linestyle  = 3 dotted
   // linestyle  = 4 dashed-dotted
   TAttLine::SetLineStyle(linestyle);
}

//______________________________________________________________________________
void TGIOS::SetLineWidth(Width_t width)
{
   // Sets the line width.
   //
   // width - the line width in pixels
   TAttLine::SetLineWidth(width);
}

//TAttFill.
//______________________________________________________________________________
void TGIOS::SetFillColor(Color_t cindex)
{
   // Sets color index "cindex" for fill areas.
   TAttFill::SetFillColor(cindex);
}

//______________________________________________________________________________
void TGIOS::SetFillStyle(Style_t style)
{
   // Sets fill area style.
   //
   // style - compound fill area interior style
   //         style = 1000 * interiorstyle + styleindex
   TAttFill::SetFillStyle(style);
}

//TAttMarker.
//______________________________________________________________________________
void TGIOS::SetMarkerColor(Color_t cindex)
{
   // Sets color index "cindex" for markers.
   TAttMarker::SetMarkerColor(cindex);
}

//______________________________________________________________________________
void TGIOS::SetMarkerSize(Float_t markersize)
{
   // Sets marker size index.
   //
   // markersize - the marker scale factor
   TAttMarker::SetMarkerSize(markersize);
}

//______________________________________________________________________________
void TGIOS::SetMarkerStyle(Style_t markerstyle)
{
   // Sets marker style.
   TAttMarker::SetMarkerStyle(markerstyle);
}

//TAttText.
//______________________________________________________________________________
void TGIOS::SetTextAlign(Short_t talign)
{
   // Sets the text alignment.
   //
   // talign = txalh horizontal text alignment
   // talign = txalv vertical text alignment
   TAttText::SetTextAlign(talign);
}

//______________________________________________________________________________
void TGIOS::SetTextColor(Color_t cindex)
{
   // Sets the color index "cindex" for text.
   TAttText::SetTextColor(cindex);
}

//______________________________________________________________________________
void TGIOS::SetTextFont(Font_t fontnumber)
{
   // Sets the current text font number.
   TAttText::SetTextFont(fontnumber);
}

//______________________________________________________________________________
void TGIOS::SetTextSize(Float_t textsize)
{
   // Sets the current text size to "textsize"
   TAttText::SetTextSize(textsize);
}

}
}
