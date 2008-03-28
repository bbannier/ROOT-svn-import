// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveCaloLegoGL.h"
#include "TEveCalo.h"
#include "TEveRGBAPalette.h"

#include "TGLIncludes.h"

#include "TGLRnrCtx.h"
#include "TGLSelectRecord.h"
#include "TGLScene.h"
#include "TGLCamera.h"
#include "TGLContext.h"

#include "TAxis.h"
#include "TObjString.h"


//______________________________________________________________________________
// OpenGL renderer class for TEveCaloLego.
//

ClassImp(TEveCaloLegoGL);

//______________________________________________________________________________
TEveCaloLegoGL::TEveCaloLegoGL() :
   TGLObject(), fM(0),
   fMinBinWidth(4),fNBinSteps(3), fBinSteps(0)
{
   // Constructor.

   fDLCache = kFALSE;

   fBinSteps = new Int_t[fNBinSteps];
   fBinSteps[0] = 1;
   fBinSteps[1] = 2;
   fBinSteps[2] = 5;
}

//______________________________________________________________________________
TEveCaloLegoGL::~TEveCaloLegoGL()
{
   // Destructor.

   delete [] fBinSteps;
}


//______________________________________________________________________________
Bool_t TEveCaloLegoGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.

   if (SetModelCheckClass(obj, TEveCaloLego::Class())) {
      fM = dynamic_cast<TEveCaloLego*>(obj);
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TEveCaloLegoGL::SetBBox()
{
   // Set bounding box.

   // !! This ok if master sub-classed from TAttBBox
   SetAxisAlignedBBox(((TEveCaloLego*)fExternalObj)->AssertBBox());
}

//______________________________________________________________________________
Bool_t TEveCaloLegoGL::ShouldDLCache(const TGLRnrCtx & /*rnrCtx*/) const
{
   if (!fM->fCacheOK) MakeDisplayList();
   return kFALSE;
}

//______________________________________________________________________________
void TEveCaloLegoGL::DLCacheDrop()
{
   for(std::map<Int_t, UInt_t>::iterator i=fDLMap.begin(); i!=fDLMap.end(); i++)
      i->second = 0;

   TGLObject::DLCacheDrop();
}

//______________________________________________________________________________
void TEveCaloLegoGL::DLCachePurge()
{
   static const TEveException eH("TEveCaloLegoGL::DLCachePurge ");

   if (fDLMap.empty()) return;

   for(std::map<Int_t, UInt_t>::iterator i=fDLMap.begin(); i!=fDLMap.end(); i++)
   {
      if (fScene) {
         fScene->GetGLCtxIdentity()->RegisterDLNameRangeToWipe(i->second, 1);
      } else {
         glDeleteLists(i->second, 1);
      }
      i->second = 0;
   }
   TGLObject::DLCachePurge();
}
//______________________________________________________________________________
void TEveCaloLegoGL::MakeQuad(Float_t x1, Float_t y1, Float_t z1, 
                              Float_t xw, Float_t yw, Float_t h) const
{ 
   //    z
   //    |
   //    |
   //    |________y
   //   /  6-------7
   //  /  /|      /|
   // x  5-------4 |
   //    | 2-----|-3
   //    |/      |/
   //    1-------0
   //

   Float_t x2 = x1+xw;
   Float_t y2 = y1+yw;
   Float_t z2 = z1+h;
   glBegin(GL_QUADS);
   {
      // bottom 0123
      glNormal3f(0, 0, -1);
      glVertex3f(x2, y2, z1);
      glVertex3f(x2, y1, z1);
      glVertex3f(x1, y1, z1);
      glVertex3f(x1, y2, z1);
      // top 4765
      glNormal3f(0, 0, 1);
      glVertex3f(x2, y2, z2);
      glVertex3f(x1, y2, z2);
      glVertex3f(x1, y1, z2);
      glVertex3f(x2, y1, z2);

      // back 0451
      glNormal3f(1, 0, 0);
      glVertex3f(x2, y2, z1);
      glVertex3f(x2, y2, z2);
      glVertex3f(x2, y1, z2);
      glVertex3f(x2, y1, z1);
      // front 3267
      glNormal3f(-1, 0, 0);
      glVertex3f(x1, y2, z1);
      glVertex3f(x1, y1, z1);
      glVertex3f(x1, y1, z2);
      glVertex3f(x1, y2, z2);

      // left  0374
      glNormal3f(0, 1, 0);
      glVertex3f(x2, y2, z1);
      glVertex3f(x1, y2, z1);
      glVertex3f(x1, y2, z2);
      glVertex3f(x2, y2, z2);
      // right 1562
      glNormal3f(0, -1, 0);
      glVertex3f(x2, y1, z1);
      glVertex3f(x2, y1, z2);
      glVertex3f(x1, y1, z2);
      glVertex3f(x1, y1, z1);
   }
   glEnd();
}

//______________________________________________________________________________
void TEveCaloLegoGL::MakeDisplayList() const
{ 
   if(fM->fCacheOK) return;
   
   fM->AssertPalette();
   TEveRGBAPalette& P = *(fM->fPalette);
   TEveCaloData::CellData_t cellData;
   Int_t   prevTower = 0;
   Float_t offset = 0;

   // ids in eta phi rng
   fM->ResetCache();
   fM->fData->GetCellList((fM->fEtaMin+fM->fEtaMax)*0.5f, fM->fEtaMax -fM->fEtaMin,
                          fM->fPhi, fM->fPhiRng, fM->fThreshold, fM->fCellList);

   Float_t scaleZ = fM->GetDefaultCellHeight()/(P.GetHighLimit()- P.GetLowLimit());  
   Int_t nSlices = fM->fData->GetNSlices(); 
   for(Int_t s=0; s<nSlices; s++) 
   {
      if (fDLMap.empty() || fDLMap[s]== 0) 
         fDLMap[s] = glGenLists(1); 

      glNewList(fDLMap[s], GL_COMPILE);
      for (UInt_t i=0; i<fM->fCellList.size(); ++i)
      {
         if (fM->fCellList[i].fSlice > s) continue;
         if (fM->fCellList[i].fTower != prevTower)
         {
            offset = 0;
            prevTower = fM->fCellList[i].fTower;
         }

         fM->fData->GetCellData(fM->fCellList[i], cellData);
         if (cellData.Value()>P.GetMinVal() && cellData.Value()<P.GetMaxVal())
         {
            Float_t z   = scaleZ*(cellData.Value() - P.GetMinVal());
            if (s == fM->fCellList[i].fSlice)
            {
               glLoadName(i);
               MakeQuad(cellData.EtaMin(), cellData.PhiMin(), offset, 
                        cellData.EtaMax()-cellData.EtaMin(), cellData.EtaMax()-cellData.EtaMin(), z);
            }
            offset += z;
         }
      }
      glEndList();
   }
   fM->fCacheOK=kTRUE;
}

//______________________________________________________________________________
inline void TEveCaloLegoGL::RnrText(const char* txt, Float_t xa, Float_t ya, Float_t za, 
                                    const GLdouble *pm, const TGLFont &font, Int_t mode) const
{
   //  3 ----- 2
   //  |       |   
   //  0 ----- 1

   glPushMatrix();
   glTranslatef(xa, ya, za);


   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);

   GLdouble x, y, z;
   gluProject(0, 0, 0, mm, pm, vp, &x, &y, &z);
   Float_t llx, lly, llz, urx, ury, urz;
   font.BBox(txt, llx, lly, llz, urx, ury, urz);

   Double_t ptb[4][3]; // 3D positions of projected text bounding-box corners.
   gluUnProject(x + llx, y + lly, z, mm, pm, vp, &ptb[0][0], &ptb[0][1], &ptb[0][2]);
   gluUnProject(x + urx, y + lly, z, mm, pm, vp, &ptb[1][0], &ptb[1][1], &ptb[1][2]);
   gluUnProject(x + urx, y + ury, z, mm, pm, vp, &ptb[2][0], &ptb[2][1], &ptb[2][2]);
   gluUnProject(x + llx, y + ury, z, mm, pm, vp, &ptb[3][0], &ptb[3][1], &ptb[3][2]);

   if (mode == 0) {
      // down for height
      glTranslatef(ptb[1][0]-ptb[2][0], ptb[1][1]-ptb[2][1], ptb[1][2]-ptb[2][2]);
      if ((ptb[0][0] < 0 && ptb[0][0] >= ptb[1][0]) ||
          (ptb[0][0] > 0 && ptb[0][0] <= ptb[1][0]))
      {
         glTranslatef(ptb[0][0]-ptb[1][0], ptb[0][1]-ptb[1][1], ptb[0][2]-ptb[1][2]);
      }
   }
   else if (mode == 1)
   {
      // down for height
      glTranslatef(ptb[1][0]-ptb[2][0], ptb[1][1]-ptb[2][1], ptb[1][2]-ptb[2][2]);
      // center at tick mark
      if (txt[0] == '-') 	 
      { 	 
         Float_t off = 0.5f * (urx-llx) / strlen(txt); 	 
         llx -= off; 	 
         urx -= off; 	 
      }
      glTranslatef(0.5f * (ptb[0][0] - ptb[1][0]),
                   0.5f * (ptb[0][1] - ptb[1][1]),
                   0.5f * (ptb[0][2] - ptb[1][2]));
   }
   else
   {
      // down for height
      {
         if(0){
            glBegin(GL_LINE_LOOP);
            glVertex3f( ptb[0][0] + (ptb[1][0]-ptb[2][0])*0.5 + ptb[0][0]-ptb[1][0], 
                        ptb[0][1] + (ptb[1][1]-ptb[2][1])*0.5 + ptb[0][1]-ptb[1][1], 
                        ptb[0][2] + (ptb[1][2]-ptb[2][2])*0.5 + ptb[0][2]-ptb[1][2]);

            glVertex3f( ptb[1][0] + (ptb[1][0]-ptb[2][0])*0.5 + ptb[0][0]-ptb[1][0],
                        ptb[1][1] + (ptb[1][1]-ptb[2][1])*0.5 + ptb[0][1]-ptb[1][1],
                        ptb[1][2] + (ptb[1][2]-ptb[2][2])*0.5 + ptb[0][2]-ptb[1][2]);

            glVertex3f( ptb[2][0] + (ptb[1][0]-ptb[2][0])*0.5 + ptb[0][0]-ptb[1][0], 
                        ptb[2][1] + (ptb[1][1]-ptb[2][1])*0.5 + ptb[0][1]-ptb[1][1], 
                        ptb[2][2] + (ptb[1][2]-ptb[2][2])*0.5 + ptb[0][2]-ptb[1][2]);

            glVertex3f( ptb[3][0] + (ptb[1][0]-ptb[2][0])*0.5 + ptb[0][0]-ptb[1][0],
                        ptb[3][1] + (ptb[1][1]-ptb[2][1])*0.5 + ptb[0][1]-ptb[1][1],
                        ptb[3][2] + (ptb[1][2]-ptb[2][2])*0.5 + ptb[0][2]-ptb[1][2]);

            glEnd();
         }
      }

      glTranslatef((ptb[1][0]-ptb[2][0])*0.5 + ptb[0][0]-ptb[1][0],
                   (ptb[1][1]-ptb[2][1])*0.5 + ptb[0][1]-ptb[1][1],
                   (ptb[1][2]-ptb[2][2])*0.5 + ptb[0][2]-ptb[1][2]);
   }

   glRasterPos3i(0, 0, 0);
   font.Render(txt);
   glPopMatrix();
}

//______________________________________________________________________________
void TEveCaloLegoGL::DrawAxis(TGLRnrCtx & rnrCtx,
                              Float_t x0, Float_t x1,
                              Float_t y0, Float_t y1) const
{
   // font for numaber and axis labels
   if (fNumFont.GetMode() == TGLFont::kUndef)
      rnrCtx.RegisterFont(16, "arial", TGLFont::kPixmap, fNumFont);
   if (fSymbolFont.GetMode() == TGLFont::kUndef)
      rnrCtx.RegisterFont(16, "symbol", TGLFont::kPixmap, fSymbolFont);

   Float_t tms = 0.1;

   // get corner closest to projected plane
   const GLdouble *pm = rnrCtx.RefCamera().RefLastNoPickProjM().CArr();
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);
   GLdouble y;
   GLdouble z[4];
   GLdouble x[4];
   gluProject(x0, y0, 0, mm, pm, vp, &x[0], &y, &z[0]);
   gluProject(x1, y0, 0, mm, pm, vp, &x[1], &y, &z[1]);
   gluProject(x1, y1, 0, mm, pm, vp, &x[2], &y, &z[2]);
   gluProject(x0, y1, 0, mm, pm, vp, &x[3], &y, &z[3]);
   /**************************************************************************/
   // XY axis   

   // get corner point closest to the eye in z-coordinate
   Float_t zt = 1.f;
   Int_t idx = 0;
   for (Int_t i=0; i<4; ++i)
   {
      if (z[i] < zt)
      {
         zt  = z[i];
         idx = i;
      }
   }

   // get location of x and y axis
   Float_t axY, ayX;
   Float_t axtX, aytY;
   switch (idx)
   {
      case 0:  
      {
         axY  = y0;  ayX = x0;
         axtX = x1; aytY = y1;
         break;
      }
      case 1: 
      {
         ayX  = x1;  axY  = y0;
         axtX = x0;  aytY = y1;  
         break;
      }
      case 2: 
      {
         ayX  = x1;  axY  = y1;
         axtX = x0;  aytY = y0;    
         break;
      }
      case 3:
      { 
         ayX  = x0;  axY = y1;
         axtX = x1;  aytY= y0;  
         break;
      }
   }

   // fill array of label locations
   Float_t lx0 = -4;
   Int_t nX = 5;

   Float_t ly0 = -2;
   Int_t nY = 5;

   // xy labels
   {
      Float_t tms = 0.1;
      fNumFont.PreRender(kFALSE);
      glPushMatrix();
      glTranslatef(0, 0, -tms*2.5);
      TGLUtil::Color(fM->fFontColor);

      RnrText("h", axtX, axY, 0, pm, fSymbolFont);
      for (Int_t i=0; i<nX; i++)
         RnrText(Form("%.0f", lx0 + 2*i), lx0 + 2*i, axY, 0, pm, fNumFont, 1);

      RnrText("F", ayX,  aytY, 0, pm, fSymbolFont);
      for (Int_t i=0; i<nY; ++i)
         RnrText(Form("%.0f", ly0 + i), ayX, ly0 + i, 0, pm, fNumFont, 1);

      glPopMatrix();
      fNumFont.PostRender();
   }

   // xy tickmarks
   TGLUtil::Color(fM->fGridColor);
   { 
      //TGLCapabilitySwitch sw_smooth(GL_LINE_SMOOTH, kTRUE);
      glBegin(GL_LINES);

      glVertex3f(x0, axY, 0);
      glVertex3f(x1, axY, 0);

      Float_t xs = 0.25f;
      Float_t xt = Int_t(x0/xs)*xs;
      while (xt < x1)
      {
         glVertex3f(xt, axY, 0);
         glVertex3f(xt, axY, ( Int_t(xt*10) % 5) ? -tms/2 : -tms);
         xt += xs;
      }

      glVertex3f(ayX, y0, 0);
      glVertex3f(ayX, y1, 0);
      Float_t ys = 0.25f;
      Float_t yt = Int_t(y0/ys)*ys;
      while (yt < y1)
      {
         glVertex3f(ayX, yt, 0);
         glVertex3f(ayX, yt, ( Int_t(yt*10) % 5) ? -tms/2 : -tms);
         yt += ys;
      }
      glEnd();
   }
   /**************************************************************************/
   // Z axis
   // x, y location of z axis
   Float_t azX, azY;
   Float_t aztX, aztY;
   
   // corner index 
   Int_t vidx = 0;
   Float_t xt = x[0];
   for (Int_t i=1; i<4; ++i)
   {
      if (xt > x[i] )
      {
         xt  = x[i];
         vidx = i;
      }
   }
   
   if( vidx == 0) {
      azX  = x0;   azY = y0;
      aztX = -tms; aztY =-tms;
   }
   else if (vidx == 1) {
      azX  = x1;      azY = y0;
      aztX = +tms; aztY = -tms;
   }
   else if (vidx == 2) {
      azX  = x1;      azY = y1;
      aztX = +tms; aztY = +tms;
   }
   else {
      azX  = x0;      azY = y1;
      aztX = -tms; aztY = +tms;
   }

   TGLUtil::Color(fM->fFontColor);

   fNumFont.PreRender(kFALSE);
   Int_t zmax = Int_t(fM->fData->GetMaxVal()/10);
   Float_t zfac = fM->fCellZScale*fM->GetDefaultCellHeight()/fM->fData->GetMaxVal();
   Float_t off = 1.9;

   for(Int_t i=1; i<=zmax; i++)
      RnrText(Form("%d",i*10), azX + aztX*off, azY + aztY*off, i*10*zfac, pm, fNumFont, 2);

   RnrText("Et[GeV]", azX + aztX*off, azY + aztY*off, (zmax+1)*10*zfac, pm, fNumFont, 2);
   fNumFont.PostRender();


   TGLUtil::Color(fM->fGridColor);

   glBegin(GL_LINES);
   glVertex3f(azX, azY, 0);
   glVertex3f(azX, azY, (zmax+1)*10*zfac);

   // z axis tick-marks
   zmax++;
   for (Int_t i=1; i<=zmax; i++)
   {
      glVertex3f(azX, azY, i*10*zfac);
      glVertex3f(azX+aztX, azY+aztY, i*10*zfac);
   }

   for (Int_t i=1; i<=zmax*10; i+=2)
   {
      glVertex3f(azX, azY, i*zfac);
      glVertex3f(azX+aztX/2, azY+aztY/2, i*zfac);
   }

   glEnd();
}

//______________________________________________________________________________
Int_t TEveCaloLegoGL::GetGridStep(Int_t axId,  const TAxis* ax,  TGLRnrCtx &rnrCtx) const
{ 
   GLdouble xp0, yp0, zp0, xp1, yp1, zp1;
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);
   const GLdouble *pm = rnrCtx.RefCamera().RefLastNoPickProjM().CArr();

   for(Int_t idx =0; idx<fNBinSteps; idx++)
   {
      if (axId == 0)
      {
         gluProject(ax->GetBinLowEdge(0), 0, 0, mm, pm, vp, &xp0, &yp0, &zp0);
         gluProject(ax->GetBinLowEdge(fBinSteps[idx]), 0, 0, mm, pm, vp, &xp1, &yp1, &zp1);
      }
      else 
      {
         gluProject(0, ax->GetBinLowEdge(0), 0, mm, pm, vp, &xp0, &yp0, &zp0);
         gluProject(0, ax->GetBinLowEdge(fBinSteps[idx]), 0, mm, pm, vp, &xp1, &yp1, &zp1);
      }

      Float_t  gap = TMath::Sqrt((xp0-xp1)*(xp0-xp1) + (yp0-yp1)*(yp0-yp1));
      if (gap>fMinBinWidth) 
      {
         return  fBinSteps[idx];
      }
   }
   return  fBinSteps[fNBinSteps-1];
}

//______________________________________________________________________________
void TEveCaloLegoGL::DrawHistBase(TGLRnrCtx &rnrCtx) const
{
   const TAxis* ax = fM->fData->GetEtaBins();
   const TAxis* ay = fM->fData->GetPhiBins();
   Float_t y0 = ay->GetBinLowEdge(0);
   Float_t y1 = ay->GetBinUpEdge(ay->GetNbins());
   Float_t x0 = ax->GetBinLowEdge(0);
   Float_t x1 = ax->GetBinUpEdge(ax->GetNbins());
   // look what is projected line density
   Int_t xs = GetGridStep(0, ax, rnrCtx);
   Int_t ys = GetGridStep(1, ay, rnrCtx);

   TGLCapabilitySwitch lights_off(GL_LIGHTING, kFALSE);
   TGLCapabilitySwitch sw_blend(GL_BLEND, kTRUE);
   TGLUtil::Color(fM->fGridColor);
   {
      glBegin(GL_LINES);
      // eta stripes
      glVertex2f(ax->GetBinLowEdge(0), y0);
      glVertex2f(ax->GetBinLowEdge(0), y1);
      for (Int_t i=1; i<ax->GetNbins(); i+=xs)
      {
         glVertex2f(ax->GetBinUpEdge(i), y0);
         glVertex2f(ax->GetBinUpEdge(i), y1);
      }
      glVertex2f(ax->GetBinUpEdge(ax->GetNbins()), y0);
      glVertex2f(ax->GetBinUpEdge(ax->GetNbins()), y1);

      // phi stipes
      glVertex2f(x0, ay->GetBinLowEdge(0));
      glVertex2f(x1, ay->GetBinLowEdge(0));
      for (Int_t i=1; i<ay->GetNbins(); i+=ys)
      {
         glVertex2f(x0, ay->GetBinUpEdge(i));
         glVertex2f(x1, ay->GetBinUpEdge(i));
      }   
      glVertex2f(x0, ay->GetBinUpEdge(ay->GetNbins()));
      glVertex2f(x1, ay->GetBinUpEdge(ay->GetNbins()));
      glEnd();
   }

  DrawAxis(rnrCtx, x0, x1, y0, y1);
}

//______________________________________________________________________________
void TEveCaloLegoGL::DirectDraw(TGLRnrCtx & rnrCtx) const
{
   // Render the calo lego-plot with OpenGL.

    DrawHistBase(rnrCtx);
 
   glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
   glPushMatrix();
   glScalef(1.f, 1.f, fM->fCellZScale);
   // cell quads
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glEnable(GL_NORMALIZE);
      glPolygonOffset(1.f, 1.f);

      glPushName(0);
      for(std::map<Int_t, UInt_t>::iterator i=fDLMap.begin(); i!=fDLMap.end(); i++)
      {
         TGLUtil::Color(fM->GetPalette()->GetDefaultColor()+i->first);
         glCallList(i->second);
      }
      glPopName();
   }
   // cell outlines
   {
      if(rnrCtx.SceneStyle() == TGLRnrCtx::kFill)
      {
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         glDisable(GL_POLYGON_OFFSET_FILL);
         TGLUtil::Color(1); 
         for(std::map<Int_t, UInt_t>::iterator i=fDLMap.begin(); i!=fDLMap.end(); i++)
            glCallList(i->second);
      }
   }
   glPopMatrix();
   glPopAttrib();
}

//______________________________________________________________________________
void TEveCaloLegoGL::ProcessSelection(TGLRnrCtx & /*rnrCtx*/, TGLSelectRecord & rec)
{
   // Processes secondary selection from TGLViewer.

   if (rec.GetN() < 2) return;
   Int_t cellID = rec.GetItem(1);
   TEveCaloData::CellData_t cellData;
   fM->fData->GetCellData(fM->fCellList[cellID], cellData);

   printf("Bin selected in slice %d \n", fM->fCellList[cellID].fSlice);
   cellData.Dump();
}
