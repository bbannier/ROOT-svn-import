#ifndef TEveGraphNode_H
#define TEveGraphNode_H

#include "TEveGeoShape.h"
#include "TAttLine.h"

class TEveGraphNode : public TEveGeoShape, public TAttLine
{
protected:
  Int_t  fSize;
  Int_t  fSizeOfAllChildren;
  Bool_t fIsDirectory;
  Bool_t fDrawParentLine;

  void position_linear(Float_t delta_x = 20, Float_t x=0, Float_t z_pos=0); // *MENU*
  void position_circle(Float_t r = 30, Float_t phi=0, Float_t z_pos=0); // *MENU*
  void position_spiral(Float_t v_r=2.5, Float_t v_phi=0.5); // *MENU*
  void position_grid(Float_t delta_x, Float_t delta_y, Int_t Nx, Float_t z_pos=0); // *MENU*

public:

 virtual void Paint(Option_t* option="");
 void SetSize(Int_t s){fSize=s;}
 Int_t GetSize () const {return fSize;}
 void SetSizeChildren(Int_t s_ch){fSizeOfAllChildren=s_ch;}
 Int_t GetSizeChildren () const {return fSizeOfAllChildren;}
 void SetflsDirectory(Int_t fD){fIsDirectory=fD;}
 Int_t GetflsDirectory() const {return fIsDirectory;}

  Bool_t GetDrawParentLine() const { return fDrawParentLine; }
  void   SetDrawParentLine(Bool_t x) { fDrawParentLine = x; StampTransBBox(); } // *MENU*

 TEveGraphNode(const char* name = "TEveGraphNode", const char* title = 0) :
   TEveGeoShape(name, title),
   fSize(0), fSizeOfAllChildren(0), fIsDirectory(kFALSE), fDrawParentLine(kTRUE)
  {}

 void PositionCircle(Float_t r, Float_t phi, Float_t z_pos);
 void PositionLinear(Float_t delta_x,Float_t x, Float_t z_pos);
 void PositionSpiral(Float_t v_r, Float_t v_phi);
 void PositionGrid(Float_t delta_x, Float_t delta_y, Int_t Nx, Float_t z_pos);
 void push_apart(Float_t t, Float_t *min=0, Float_t* max=0);
 void push_by_dist(Float_t max_dist, Bool_t use_size=kFALSE);
 void minimal_dist(Float_t t, Float_t mindist, Bool_t shrink=kTRUE, Bool_t animate=kTRUE);
 Float_t how_much_space (Float_t space_per_child);
 Float_t how_much_space_circle (Float_t space_per_child);
 Float_t how_much_space_grid (Float_t space_per_child);
 void set_children_color (Color_t ch);// *MENU*
 void set_children_color_recursively(Color_t ch);// *MENU*
 ClassDef(TEveGraphNode, 0);
};

#endif
