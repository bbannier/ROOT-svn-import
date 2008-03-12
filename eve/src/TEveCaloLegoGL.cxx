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
   printf("~xTEveCaloLegoGL() \n");
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
inline void TEveCaloLegoGL::RnrText(const char* txt, Float_t xa, Float_t ya, const GLdouble *pm) const
{   
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp); 

   fX[0][0] = xa;
   fX[0][1] = ya;
   fX[0][2] = 0;
   GLdouble x, y, z;
   gluProject(fX[0][0], fX[0][1], fX[0][2], mm, pm, vp, &x, &y, &z);
   Float_t bbox[6];
   fNumFont.BBox(txt, bbox[0], bbox[1], bbox[2], bbox[3], bbox[4], bbox[5]);
   if (txt[0] == '-')
   {
      Float_t off = 0.5f * (bbox[3]-bbox[0]) / strlen(txt);
      bbox[0] -= off;
      bbox[3] -= off;
   }
   gluUnProject(x + bbox[0], y + bbox[1], z, mm, pm, vp, &fX[0][0], &fX[0][1], &fX[0][2]);
   gluUnProject(x + bbox[3], y + bbox[1], z, mm, pm, vp, &fX[1][0], &fX[1][1], &fX[1][2]);
   gluUnProject(x + bbox[3], y + bbox[4], z, mm, pm, vp, &fX[2][0], &fX[2][1], &fX[2][2]);
   gluUnProject(x + bbox[0], y + bbox[4], z, mm, pm, vp, &fX[3][0], &fX[3][1], &fX[3][2]);

   glPushMatrix();
   // down for height
   glTranslatef(fX[1][0]-fX[2][0], fX[1][1]-fX[2][1], fX[1][2]-fX[2][2]); 
   // left for the middle of tick mark 
   glTranslatef((fX[0][0]-fX[1][0])*0.5f, (fX[0][1]-fX[1][1])*0.5f, (fX[0][2]-fX[1][2])*0.5f);

   // debug
   if (0)
   {
      glBegin(GL_LINE_LOOP);
      glVertex3dv(fX[0]);
      glVertex3dv(fX[1]);
      glVertex3dv(fX[2]);
      glVertex3dv(fX[3]);
      glEnd();
   }
   // translate to locatio
   glTranslatef(fX[0][0], fX[0][1], fX[0][2]); 

   glRasterPos3i(0, 0, 0);
   fNumFont.Render(txt);
   glPopMatrix();
}

//______________________________________________________________________________
void TEveCaloLegoGL::DrawTitle(TGLRnrCtx & rnrCtx ) const
{
   if (fTitleFont.GetMode() == TGLFont::kUndef) 
   {
      TObjArray* farr = TGLFontManager::GetFontFileArray();
      TIter next(farr);
      TObjString* os;
      Int_t cnt = 0;
      while ((os = (TObjString*) next()) != 0)
      {
         if (os->GetString() == "comic")
            break;
         cnt++;
      } 
      rnrCtx.RegisterFont(22, cnt, TGLFont::kPixmap, fTitleFont);      
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
void TEveCaloLegoGL::DrawAxis(TGLRnrCtx & rnrCtx,
                              Float_t x0, Float_t x1,
                              Float_t y0, Float_t y1) const
{
   if (fNumFont.GetMode() == TGLFont::kUndef)
   {
      TObjArray* farr = TGLFontManager::GetFontFileArray();
      TIter next(farr);
      TObjString* os;
      Int_t cnt = 0;
      while ((os = (TObjString*) next()) != 0)
      {
         if (os->GetString() == "comic")
            break;
         cnt++;
      } 
      rnrCtx.RegisterFont(12, cnt, TGLFont::kPixmap, fNumFont);   
   }

   // get corner closest to projected plane

   const GLdouble *pm = rnrCtx.RefCamera().RefLastNoPickProjM().CArr(); 
   GLdouble mm[16];
   GLint    vp[4];
   glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
   glGetIntegerv(GL_VIEWPORT, vp); 
   Float_t axY, ayX;
   GLdouble x, y;
   GLdouble z[4];
   gluProject(x0, y0, 0, mm, pm, vp, &x, &y, &z[0]);
   gluProject(x1, y0, 0, mm, pm, vp, &x, &y, &z[1]);
   gluProject(x1, y1, 0, mm, pm, vp, &x, &y, &z[2]);
   gluProject(x0, y1, 0, mm, pm, vp, &x, &y, &z[3]);

   Float_t zt = 1.f;
   Int_t idx = 0;
   for(Int_t i=0; i<4; i++)
   {
      if(z[i] < zt) {
         zt = z[i];
         idx = i;
      }
   }

   switch (idx) {
      case 0:
         axY = y0;
         ayX = x0;
         break;
      case 1:
         ayX = x1;
         axY = y0;
         break;
      case 2:
         ayX = x1;
         axY = y1;
         break;
      case 3:
         ayX = x0;
         axY = y1;
         break;
   }

   // labels
   Int_t ntmX = 2;
   Float_t xs = 2;
   Int_t nX = ntmX*2+1;
   Float_t* vX = new Float_t[nX];
   for(Int_t i=0; i<nX; i++)
      vX[i] = -xs*ntmX + i*xs;

   Int_t ntmY = 3;
   Float_t ys = 1;
   Int_t nY = ntmY*2+1;
   Float_t* vY = new Float_t[nY];
   for(Int_t i=0; i<nY; i++)
      vY[i] = -ys*ntmY + i*ys;

   

   Float_t tms = 0.1;
   Float_t tmOff = 1.5*tms; 

   fNumFont.PreRender(kFALSE);
   glPushMatrix();
   glTranslatef(0, 0, -tms -tmOff);


   for(Int_t i=0; i<nX; i++)
      RnrText(Form("%.0f", vX[i]), vX[i], axY, pm);

   for(Int_t i=0; i<nY; i++)
      RnrText(Form("%.0f", vY[i]), ayX, vY[i], pm);

   glPopMatrix();
   fNumFont.PostRender();
   

   // tickmarks
   glBegin(GL_LINES);
   glVertex3f(x0, axY, 0);
   glVertex3f(x1, axY, 0);
   for(Int_t i=0; i<nX; i++)
   {
      glVertex3f(vX[i], axY, 0);
      glVertex3f(vX[i], axY, -tms);
   } 
   glVertex3f(ayX, y0, 0);
   glVertex3f(ayX, y1, 0);
   for(Int_t i=0; i<nY; i++)
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
   // grid lines   
   const TAxis* ax = fM->GetData()->GetEtaBins();
   const TAxis* ay = fM->GetData()->GetPhiBins();  
   Float_t y0 = ay->GetBinLowEdge(0);
   Float_t y1 = ay->GetBinUpEdge(ay->GetNbins());
   Float_t x0 = ax->GetBinLowEdge(0);
   Float_t x1 = ax->GetBinUpEdge(ax->GetNbins());
   {
      Float_t a = 0.2;
      glColor3f(a, a, a);
      TGLCapabilitySwitch lights_on(GL_LIGHTING, kTRUE);

      glBegin(GL_LINES);
      glVertex2f(ax->GetBinLowEdge(0), y0); 
      glVertex2f(ax->GetBinLowEdge(0), y1);
      for(Int_t i=0; i<=ax->GetNbins(); i++)
      {
         glVertex2f(ax->GetBinUpEdge(i), y0); 
         glVertex2f(ax->GetBinUpEdge(i), y1);
      }
      glVertex2f(x0, ay->GetBinLowEdge(0)); 
      glVertex2f(x1, ay->GetBinLowEdge(0));
      for(Int_t i=0; i<=ay->GetNbins(); i++)
      {
         glVertex2f(x0, ay->GetBinUpEdge(i)); 
         glVertex2f(x1, ay->GetBinUpEdge(i));
      }
      glEnd();
   }

   glColor3f(1, 1, 1);
   DrawTitle(rnrCtx);
   DrawAxis(rnrCtx, x0, x1, y0, y1);
   
   // cells
   fM->AssertPalette();
   if (fM->fCacheOK == kFALSE)
   {
      fM->ResetCache();
      fM->fData->GetCellList((fM->fEtaMin+fM->fEtaMax)*0.5f, fM->fEtaMax -fM->fEtaMin,
                             fM->fPhi, fM->fPhiRng, fM->fThreshold, fM->fCellList);
      fM->fCacheOK= kTRUE;
   }
   TGLCapabilitySwitch lights_on(GL_LIGHTING, kTRUE);
   TEveCaloData::CellData_t cellData;
   Float_t towerH = 0;
   Bool_t  visible = kFALSE;
   Int_t   prevTower = 0;
   Float_t offset = 0;
   if (rnrCtx.SecSelection()) glPushName(0);
   for(UInt_t i=0; i<fM->fCellList.size(); i++)
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
