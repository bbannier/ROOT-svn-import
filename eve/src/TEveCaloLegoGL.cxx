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

#include "TGLRnrCtx.h"
#include "TGLSelectRecord.h"
#include "TGLIncludes.h"
#include "TGLCamera.h"

#include "TAxis.h"
#include "TObjString.h"


//______________________________________________________________________________
// OpenGL renderer class for TEveCaloLego.
//

ClassImp(TEveCaloLegoGL);

//______________________________________________________________________________
TEveCaloLegoGL::TEveCaloLegoGL() :
   TGLObject(), fM(0)
{
   // Constructor.
   fDLCache = kFALSE;
}

//______________________________________________________________________________
TEveCaloLegoGL::~TEveCaloLegoGL()
{
   // Destructor.

}

/******************************************************************************/

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

/******************************************************************************/

//______________________________________________________________________________
Float_t TEveCaloLegoGL::RenderCell(const TEveCaloData::CellData_t &cellData, Float_t towerH, Float_t offset ) const
{
   Float_t x  = cellData.EtaMin();
   Float_t xw = cellData.EtaMax() - cellData.EtaMin();
   Float_t y  = cellData.PhiMin();
   Float_t yw = cellData.PhiMax() - cellData.PhiMin();
   Float_t z  = offset;
   Float_t h  = towerH;

   glBegin(GL_QUADS);
   // bottom 1230
   glNormal3f(0, 0, -1);
   glVertex3f(x,    y,    z  );
   glVertex3f(x,    y+yw, z  );
   glVertex3f(x+xw, y+yw, z  );
   glVertex3f(x+xw, y,    z  );
   // top 4765
   glNormal3f(0, 0, 1);
   glVertex3f(x+xw, y,    z+h);
   glVertex3f(x+xw, y+yw, z+h);
   glVertex3f(x,    y+yw, z+h);
   glVertex3f(x,    y,    z+h);
   // back:   0451
   glNormal3f(0, 1, 0);
   glVertex3f(x+xw, y,    z  );
   glVertex3f(x+xw, y,    z+h);
   glVertex3f(x   , y,    z+h);
   glVertex3f(x   , y,    z  );
   // front 3267
   glNormal3f(0, -1, 0);
   glVertex3f(x+xw, y+yw, z  );
   glVertex3f(x,    y+yw, z  );
   glVertex3f(x,    y+yw, z+h);
   glVertex3f(x+xw, y+yw, z+h);
   // left  0374
   glNormal3f(-1, 0, 0);
   glVertex3f(x+xw, y,    z  );
   glVertex3f(x+xw, y+yw, z  );
   glVertex3f(x+xw, y+yw, z+h);
   glVertex3f(x+xw, y   , z+h);
   // right 1562
   glNormal3f(1, 0, 0);
   glVertex3f(x,    y,    z  );
   glVertex3f(x,    y,    z+h);
   glVertex3f(x,    y+yw, z+h);
   glVertex3f(x,    y+yw, z  );
   glEnd();

   return offset + towerH;
}

//______________________________________________________________________________
void TEveCaloLegoGL::DrawTitle(TGLRnrCtx & rnrCtx ) const
{
   if (fTitleFont.GetMode() == TGLFont::kUndef)
   {
      TObjArray* farr = TGLFontManager::GetFontFileArray();
      TIter next(farr);
      rnrCtx.RegisterFont(22, "arial", TGLFont::kPixmap, fTitleFont);
   }

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   fTitleFont.PreRender(kFALSE);
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);

   Float_t bbox[6];
   fTitleFont.BBox(fM->GetTitle(), bbox[0], bbox[1], bbox[2], bbox[3], bbox[4], bbox[5]);
   bbox[3] += fTitleFont.GetSize()*2;
   bbox[4] += fTitleFont.GetSize()*2;
   Float_t xf = (vp[2]-bbox[3])/(vp[2]-vp[0]);
   Float_t yf = (vp[3]-bbox[4])/(vp[3]-vp[1]);
   glPushMatrix();
   glTranslatef( xf*2 -1, yf*2 -1, 0);
   glRasterPos3i(0,0, 0);
   fTitleFont.Render(fM->GetTitle());
   glPopMatrix();
   fTitleFont.PostRender();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//______________________________________________________________________________
inline void TEveCaloLegoGL::RnrText(const char* txt, Float_t xa, Float_t ya, const GLdouble *pm, Bool_t isNum) const
{
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);

   Double_t ptb[4][3]; // 3D positions of projected text bounding-box corners.
   ptb[0][0] = xa;
   ptb[0][1] = ya;
   ptb[0][2] = 0;
   GLdouble x, y, z;
   gluProject(ptb[0][0], ptb[0][1], ptb[0][2], mm, pm, vp, &x, &y, &z);
   Float_t llx, lly, llz, urx, ury, urz;
   if (isNum)
      fNumFont.BBox(txt, llx, lly, llz, urx, ury, urz);
   else 
      fSymbolFont.BBox(txt, llx, lly, llz, urx, ury, urz);

   gluUnProject(x + llx, y + lly, z, mm, pm, vp, &ptb[0][0], &ptb[0][1], &ptb[0][2]);
   gluUnProject(x + urx, y + lly, z, mm, pm, vp, &ptb[1][0], &ptb[1][1], &ptb[1][2]);
   gluUnProject(x + urx, y + ury, z, mm, pm, vp, &ptb[2][0], &ptb[2][1], &ptb[2][2]);
   gluUnProject(x + llx, y + ury, z, mm, pm, vp, &ptb[3][0], &ptb[3][1], &ptb[3][2]);

   glPushMatrix();
   // down for height
   glTranslatef(ptb[1][0]-ptb[2][0], ptb[1][1]-ptb[2][1], ptb[1][2]-ptb[2][2]);
   // translate to location
   glTranslatef(ptb[0][0], ptb[0][1], ptb[0][2]);

   if (isNum)
   {
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
      glRasterPos3i(0, 0, 0);
      fNumFont.Render(txt);
   }
   else
   {
      if ((ptb[0][0] < 0 && ptb[0][0] >= ptb[1][0]) ||
          (ptb[0][0] > 0 && ptb[0][0] <= ptb[1][0]))
      {
         glTranslatef(ptb[0][0]-ptb[1][0], ptb[0][1]-ptb[1][1], ptb[0][2]-ptb[1][2]);
      }
      glRasterPos3i(0, 0, 0);
      fSymbolFont.Render(txt);
   }
   glPopMatrix();
}

//______________________________________________________________________________
void TEveCaloLegoGL::DrawAxis(TGLRnrCtx & rnrCtx,
                              Float_t x0, Float_t x1,
                              Float_t y0, Float_t y1) const
{
   if (fNumFont.GetMode() == TGLFont::kUndef)
      rnrCtx.RegisterFont(12, "arial", TGLFont::kPixmap, fNumFont);
   if (fSymbolFont.GetMode() == TGLFont::kUndef)
      rnrCtx.RegisterFont(20, "symbol", TGLFont::kPixmap, fSymbolFont);


   // get corner closest to projected plane
   const GLdouble *pm = rnrCtx.RefCamera().RefLastNoPickProjM().CArr();
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp);
   GLdouble x, y;
   GLdouble z[4];
   gluProject(x0, y0, 0, mm, pm, vp, &x, &y, &z[0]);
   gluProject(x1, y0, 0, mm, pm, vp, &x, &y, &z[1]);
   gluProject(x1, y1, 0, mm, pm, vp, &x, &y, &z[2]);
   gluProject(x0, y1, 0, mm, pm, vp, &x, &y, &z[3]);

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

   Int_t ntmX = 2;
   Float_t xs = 2;
   Float_t tms = 0.1;
   Float_t tmOff = 1.5*tms;
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

   Int_t nX = ntmX*2+1;
   Float_t* vX = new Float_t[nX];
   for (Int_t i=0; i<nX; ++i)
      vX[i] = -xs*ntmX + i*xs;

   Int_t ntmY = 2;
   Float_t ys = 1;
   Int_t nY = ntmY*2+1;
   Float_t* vY = new Float_t[nY];
   for (Int_t i=0; i<nY; ++i)
      vY[i] = -ys*ntmY + i*ys;

   fNumFont.PreRender(kFALSE);
   glPushMatrix();
   glTranslatef(0, 0, -tms -tmOff);

   RnrText("h", axtX, axY , pm, kFALSE);
   RnrText("f", ayX,  aytY, pm, kFALSE);

   for (Int_t i=0; i<nX; ++i)
      RnrText(Form("%.0f", vX[i]), vX[i], axY, pm, kTRUE);
   for (Int_t i=0; i<nY; ++i)
      RnrText(Form("%.0f", vY[i]), ayX, vY[i], pm, kTRUE);
   glPopMatrix();
   fNumFont.PostRender();


   // tickmarks
   glBegin(GL_LINES);
   glVertex3f(x0, axY, 0);
   glVertex3f(x1, axY, 0);
   for (Int_t i=0; i<nX; ++i)
   {
      glVertex3f(vX[i], axY, 0);
      glVertex3f(vX[i], axY, -tms);
   }
   glVertex3f(ayX, y0, 0);
   glVertex3f(ayX, y1, 0);
   for (Int_t i=0; i<nY; ++i)
   {
      glVertex3f(ayX, vY[i], 0);
      glVertex3f(ayX, vY[i], -tms);
   }
   glEnd();

   delete [] vX;
   delete [] vY;
}

//______________________________________________________________________________
void TEveCaloLegoGL::DirectDraw(TGLRnrCtx & rnrCtx) const
{
   // Render the calo lego-plot with OpenGL.

   // grid lines
   const TAxis* ax = fM->GetData()->GetEtaBins();
   const TAxis* ay = fM->GetData()->GetPhiBins();
   Float_t y0 = ay->GetBinLowEdge(0);
   Float_t y1 = ay->GetBinUpEdge(ay->GetNbins());
   Float_t x0 = ax->GetBinLowEdge(0);
   Float_t x1 = ax->GetBinUpEdge(ax->GetNbins());
   {
      Float_t a = 0.5;
      TGLUtil::Color3f(a, a, a);
      TGLCapabilitySwitch lights_off(GL_LIGHTING, kFALSE);

      glBegin(GL_LINES);
      glVertex2f(ax->GetBinLowEdge(0), y0);
      glVertex2f(ax->GetBinLowEdge(0), y1);
      for (Int_t i=0; i<=ax->GetNbins(); ++i)
      {
         glVertex2f(ax->GetBinUpEdge(i), y0);
         glVertex2f(ax->GetBinUpEdge(i), y1);
      }
      glVertex2f(x0, ay->GetBinLowEdge(0));
      glVertex2f(x1, ay->GetBinLowEdge(0));
      for (Int_t i=0; i<=ay->GetNbins(); ++i)
      {
         glVertex2f(x0, ay->GetBinUpEdge(i));
         glVertex2f(x1, ay->GetBinUpEdge(i));
      }
      glEnd();
   }

   TGLUtil::Color3f(1, 1, 1);
   DrawTitle(rnrCtx);
   DrawAxis(rnrCtx, x0, x1, y0, y1);

   // cells
   fM->AssertPalette();
   if (fM->fCacheOK == kFALSE)
   {
      fM->ResetCache();
      fM->fData->GetCellList((fM->fEtaMin+fM->fEtaMax)*0.5f, fM->fEtaMax -fM->fEtaMin,
                             fM->fPhi, fM->fPhiRng, fM->fThreshold, fM->fCellList);
      fM->fCacheOK = kTRUE;
   }

   TEveCaloData::CellData_t cellData;
   Float_t towerH = 0;
   Bool_t  visible = kFALSE;
   Int_t   prevTower = 0;
   Float_t offset = 0;
   if (rnrCtx.SecSelection()) glPushName(0);
   for (UInt_t i=0; i<fM->fCellList.size(); ++i)
   {
      fM->fData->GetCellData(fM->fCellList[i], cellData);
      if (fM->fCellList[i].fTower != prevTower)
      {
         offset = 0;
         prevTower = fM->fCellList[i].fTower;
      }

      fM->SetupColorHeight(cellData.Value(), fM->fCellList[i].fSlice, towerH, visible);
      if (visible)
      {
         if (rnrCtx.SecSelection()) glLoadName(i);
         offset = RenderCell(cellData, towerH, offset);
      }
   }
   if (rnrCtx.SecSelection()) glPopName();
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
