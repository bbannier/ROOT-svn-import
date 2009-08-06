#include "TEveGraphNode.h"
#include "TEveManager.h"
#include "TEveTrans.h"
#include "TGLViewer.h"
#include "TEveVSDStructs.h"
#include "TGeoTube.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TVirtualViewer3D.h"

#include "TSystem.h"
#include "iostream"
#include "TMath.h"

using namespace std;

//______________________________________________________________________________
void TEveGraphNode::Paint(Option_t* /*option*/)
{
   // Paint this object. Only direct rendering is supported.

   static const TEveException eH("TEveDigitSet::Paint ");

   TBuffer3D buff(TBuffer3DTypes::kGeneric);

   // Section kCore
   buff.fID           = this;
   buff.fColor        = GetMainColor();
   buff.fTransparency = GetMainTransparency();
   RefMainTrans().SetBuffer3D(buff);
   buff.SetSectionsValid(TBuffer3D::kCore);

   Int_t reqSections = gPad->GetViewer3D()->AddObject(buff);
   if (reqSections != TBuffer3D::kNone)
      Error(eH, "only direct GL rendering supported.");
}

void TEveGraphNode::push_apart(Float_t t, Float_t *min, Float_t* max)
{
  TEveVector* v = new TEveVector[NumChildren()];

  Bool_t first = kTRUE;
  Int_t ci = 0;
  for (TEveElement::List_i i =BeginChildren(); i != EndChildren(); ++i)
  {
    TEveVector ri((*i)->RefMainTrans().ArrT());
      Int_t cj=ci;
     for (TEveElement::List_i j = i; j != EndChildren(); ++j, ++cj)
     {
        
        if (*i==*j) continue;
       
        TEveVector rj((*j)->RefMainTrans().ArrT());
        TEveVector rij=ri-rj;
        Float_t dist=rij.Mag();
        if (first)
        {
         if (min) *min = dist;
         if (max) *max = dist;
         first = kFALSE;
        }
        else
        {
         if (min && dist < *min) *min = dist;
         if (max && dist > *max) *max = dist;
        }
        rij*=pow(dist,1.0f/3);
        v[ci] +=rij;
        v[cj] -=rij;
        cout<<ci<<" "<<cj<<" ";
        rij.Dump();
        
      
     }
     ++ci;
  }
  
  ci = 0;
  
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    TEveElement *el = *i;
    v[ci]*= t;
    el->RefMainTrans().Move3PF(v[ci].fX,v[ci].fY,v[ci].fZ); // size or size-of-all as diminishing factor
    el->StampTransBBox();
    ++ci; 
  }
  
  delete [] v;
  
  gEve->Redraw3D();
 
}
void TEveGraphNode::push_by_dist(Float_t max_dist, Bool_t use_size)
{
  TEveVector* v = new TEveVector[NumChildren()];

  Int_t ci = 0;
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    TEveVector ri((*i)->RefMainTrans().ArrT());
      Int_t cj=ci;
     for (TEveElement::List_i j = i; j != EndChildren(); ++j, ++cj)
     {
        
        if (*i==*j)continue;
       


        
        TEveVector rj((*j)->RefMainTrans().ArrT());
        TEveVector rij=ri-rj;
        Float_t dist=rij.Mag();
        rij*=pow(dist,1.0f/3);
        v[ci] +=rij;
        v[cj] -=rij;
        cout<<ci<<" "<<cj<<" ";
        rij.Dump();
        
      
     }
     ++ci;
  }
  
  if (use_size)
  {
    ci = 0;
    for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
    {
      v[ci] *= 1.0f/((TEveGraphNode*) *i)->GetSize();
      ++ci;
    }
  }
  
  ci = 0;
  Float_t max_force = v[0].Mag();
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    if (v[ci].Mag() > max_force)
    {
      max_force = v[ci].Mag();
    }
    ++ci;
  }

  Float_t t = max_dist/max_force;

  ci = 0;  
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    TEveElement *el = *i;
    v[ci] *= t;
    el->RefMainTrans().Move3PF(v[ci].fX,v[ci].fY,v[ci].fZ); // size or size-of-all as diminishing factor
    el->StampTransBBox();
    ++ci; 
  }
  
  delete [] v;
  
  gEve->Redraw3D();
 
}
void TEveGraphNode::minimal_dist(Float_t t, Float_t mindist, Bool_t shrink, Bool_t animate)
{
  Float_t min,max;
  Int_t   npic = 0;
  push_apart(0, &min, &max);
  if (min < mindist)
  {
    t = fabs(t);
    while (min < mindist)
    {
      push_apart(t, &min, &max);
      if (animate) {
        gSystem->ProcessEvents();
        gEve->GetDefaultGLViewer()->SavePicture(Form("pepe-%04d.jpg", npic++));
      }
    }
  }
  else if (shrink)
  {
    t = - fabs(t);
    while (min > mindist)
    {
      push_apart(t, &min, &max);
      if (animate)
        gSystem->ProcessEvents();
    }
  }
  cout<<"That's the minimal distance!"<<endl;

}
void TEveGraphNode::position_linear(Float_t delta_x,Float_t x, Float_t z_pos)
{
  
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    TEveElement *el = *i;

    el->RefMainTrans().SetPos(x, 0, z_pos);
    el->StampTransBBox();

    TEveGeoShape *shp = dynamic_cast<TEveGeoShape*>(el);
    cout << "Position " << x << " " << el->GetElementName() << " el=" << el << " shp=" << shp << endl;

    x += delta_x;

  }
  gEve->Redraw3D(kTRUE);
}

void TEveGraphNode::position_circle(Float_t r, Float_t phi, Float_t z_pos)
{
  Float_t delta_phi = TMath::TwoPi() / NumChildren();

  for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
  {
    TEveElement *el = *i;

    el->RefMainTrans().SetPos(r*TMath::Cos(phi), r*TMath::Sin(phi), z_pos);
    el->StampTransBBox();

    TEveGeoShape *shp = dynamic_cast<TEveGeoShape*>(el);
    cout << "Phi " << phi << " " << el->GetElementName() << " el=" << el << " shp=" << shp << endl;

    phi += delta_phi;
  }
  gEve->Redraw3D(kTRUE);
}


void TEveGraphNode::position_spiral(Float_t v_r, Float_t v_phi)
{
  Float_t x = 0, y = 0, t = 0, dt = 0.1, xn = 0, yn = 0, dx, dy;

  TEveElement::List_i i = BeginChildren();
  TEveElement *el = *i;
  el->RefMainTrans().SetPos(xn, yn, 0);
  el->StampTransBBox();

  while (i != EndChildren())
  {
    TEveElement *el = *i;

    do
    {
      t += dt;
      xn = v_r*t * TMath::Cos(v_phi*t);
      yn = v_r*t * TMath::Sin(v_phi*t);
      dx = xn - x;
      dy = yn - y;
    }
    while (dx*dx + dy*dy < 400);

    el->RefMainTrans().SetPos(xn, yn, 0);
    el->StampTransBBox();

    x = xn;
    y = yn;

    ++i;
  }
  gEve->Redraw3D(kTRUE);
}

void TEveGraphNode::position_grid(Float_t delta_x, Float_t delta_y, Int_t Nx, Float_t z_pos)
{
  Float_t x=0, y=0;
  Int_t j=0;
  for (TEveElement::List_i i = BeginChildren(); i != EndChildren();++i)
  {
    TEveElement *el=*i;
    el->RefMainTrans().SetPos(x,y, z_pos);
    el->StampTransBBox();
    TEveGeoShape *shp = dynamic_cast<TEveGeoShape*>(el);
    cout << "Position " << x << " " << el->GetElementName() << " el=" << el<< " shp=" << shp << endl;
    x += delta_x;
    ++j;

    if (j==Nx)
    {
      y+=delta_y;
      x=0;
      j=0;
    }

  }
  gEve->Redraw3D(kTRUE);
}
int TEveGraphNode::how_much_space (int space_per_child)
{
 Int_t max_space=0;
 for (TEveElement::List_i i=BeginChildren(); i != EndChildren(); ++i)
{
     TEveGraphNode *gm=dynamic_cast<TEveGraphNode*>(*i);
    Int_t space=gm->how_much_space(space_per_child);
     if (space>max_space)
     max_space=space;
}
 if (NumChildren()==0)
 return space_per_child;
 position_linear(max_space, -0.5 * max_space * (NumChildren()-1), max_space);
 return max_space * NumChildren();
}

int TEveGraphNode::how_much_space_circle (int space_per_child)
{
 Int_t max_space=0;
 for (TEveElement::List_i i=BeginChildren(); i != EndChildren(); ++i)
{
     TEveGraphNode *gm=dynamic_cast<TEveGraphNode*>(*i);
    Int_t space=gm->how_much_space_circle(space_per_child);
     if (space>max_space)
     max_space=space;
}
 if (NumChildren()==0)
 return space_per_child;
 position_circle((max_space * NumChildren())/(2*3.14), 0, max_space);
 return (3*max_space * NumChildren())/(2*3.14);
}
int TEveGraphNode::how_much_space_grid (int space_per_child)
{
   Int_t max_space=0;
   for (TEveElement::List_i i=BeginChildren(); i != EndChildren(); ++i)
  {
    TEveGraphNode *gm=dynamic_cast<TEveGraphNode*>(*i);
    Int_t space=gm->how_much_space_grid(space_per_child);
    if (space>max_space)
    max_space=space;
  }
   if (NumChildren()==0)
   return space_per_child;
   position_grid(max_space, max_space, 2, max_space);
    return max_space * NumChildren();
}

void TEveGraphNode::set_children_color (Color_t ch)
{
    for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
    {
     TEveElement *el=*i;
     el->SetMainColor(ch);

    }
    gEve->Redraw3D(kTRUE);
}
void TEveGraphNode::set_children_color_recursively(Color_t ch)
{
    for (TEveElement::List_i i = BeginChildren(); i != EndChildren(); ++i)
    {
     TEveElement *el=*i;
     el->SetMainColor(ch);
     TEveGraphNode *gm=dynamic_cast<TEveGraphNode*>(*i);
     gm->set_children_color_recursively (ch);
 
    }
     gEve->Redraw3D(kTRUE);
    
     
}


