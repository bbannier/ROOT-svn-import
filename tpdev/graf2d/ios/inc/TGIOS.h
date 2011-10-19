#ifndef ROOT_TGIOS
#define ROOT_TGIOS

#ifndef ROOT_TVirtualX
#include "TVirtualX.h"
#endif

//
//TVirtualX for iOS. No window management, no graphics, just have
//to implement correctly functions from TAttXXX base classes 
//(TVirtualX intentionally overrides some of them and has
//empty implementations). But on iOS I do not have TGWin32 or
//TGX11 and graphic primitives try to use gVirtualX to pass
//different attributes like color, line width, etc. to the 
//painting code.
//

namespace ROOT {
namespace iOS {

class TGIOS : public TVirtualX {
public:
   TGIOS();
   TGIOS(const char *name, const char *title);
   
   void SetLineColor(Color_t cindex);
   void SetLineStyle(Style_t linestyle);
   void SetLineWidth(Width_t width);
   void SetFillColor(Color_t cindex);
   void SetFillStyle(Style_t style);
   void SetMarkerColor(Color_t cindex);
   void SetMarkerSize(Float_t markersize);
   void SetMarkerStyle(Style_t markerstyle);
   void SetTextAlign(Short_t talign);
   void SetTextColor(Color_t cindex);
   void SetTextFont(Font_t fontnumber);
   void SetTextSize(Float_t textsize);
private:
   TGIOS(const TGIOS &rhs);
   TGIOS &operator = (const TGIOS &rhs);
};

}
}

#endif
