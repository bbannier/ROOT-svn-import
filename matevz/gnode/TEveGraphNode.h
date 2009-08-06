#ifndef TEveGraphNode_H
#define TEveGraphNode_H

#include "TEveGeoShape.h"
class TEveGraphNode : public TEveGeoShape
{
protected:
  Int_t  fSize;
  Int_t  fSizeOfAllChildren;
  Bool_t fIsDirectory;

public:

 virtual void Paint(Option_t* option="");
 void SetSize(Int_t s){fSize=s;}
 Int_t GetSize () const {return fSize;}
 void SetSizeChildren(Int_t s_ch){fSizeOfAllChildren=s_ch;}
 Int_t GetSizeChildren () const {return fSizeOfAllChildren;}
 void SetflsDirectory(Int_t fD){fIsDirectory=fD;}
 Int_t GetflsDirectory() const {return fIsDirectory;}
 TEveGraphNode(const char* name = "TEveGraphNode", const char* title = 0) : TEveGeoShape(name, title){}
 
 void position_linear(Float_t delta_x = 20, Float_t x=0, Float_t z_pos=0); // *MENU*
 void position_circle(Float_t r = 30, Float_t phi=0, Float_t z_pos=0); // *MENU*
 void position_spiral(Float_t v_r=2.5, Float_t v_phi=0.5); // *MENU*
 void position_grid(Float_t delta_x, Float_t delta_y, Int_t Nx, Float_t z_pos=0); // *MENU*
 void push_apart(Float_t t, Float_t *min=0, Float_t* max=0);
 void push_by_dist(Float_t max_dist, Bool_t use_size=kFALSE);
 void minimal_dist(Float_t t, Float_t mindist, Bool_t shrink=kTRUE, Bool_t animate=kTRUE);
 int how_much_space (int space_per_child);
 int how_much_space_circle (int space_per_child);
 int how_much_space_grid (int space_per_child);
 void set_children_color (Color_t ch);// *MENU*
 void set_children_color_recursively(Color_t ch);// *MENU*
 ClassDef(TEveGraphNode, 0);
};

#endif
