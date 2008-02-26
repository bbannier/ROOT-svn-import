#include "TEveProjections.h"

const char* histFile = "http://amraktad.web.cern.ch/amraktad/cms_calo_hist.root";
const char* geom_file_name = "http://root.cern.ch/files/alice_ESDgeometry.root";

void cms_calo()
{
   TFile::SetCacheFileDir(".");
   TEveManager::Create();
   gEve->GetSelection()->SetPickToSelect(1);
   gEve->GetHighlight()->SetPickToSelect(0);

   // geom
   TFile* geom = TFile::Open(geom_file_name, "CACHEREAD");
   if (!geom)
      return;
   TEveGeoShapeExtract* gse = (TEveGeoShapeExtract*) geom->Get("Gentle");
   TEveGeoShape* gsre = TEveGeoShape::ImportShapeExtract(gse, 0);
   geom->Close();
   delete geom;

   // palette
   gStyle->SetPalette(1, 0);
   TEveRGBAPalette* pal = new TEveRGBAPalette(0, 100);
   pal->SetLimits(0, 50);
   pal->SetDefaultColor((Color_t)4);
   pal->SetShowDefValue(kFALSE);

   // calorimeter
   TFile* hf = TFile::Open(histFile, "CACHEREAD");
   TH2F* hcalHist = (TH2F*)hf->Get("hcalLego");
   TH2F* ecalHist = (TH2F*)hf->Get("ecalLego");
   TEveCaloDataHist* data = new TEveCaloDataHist();
   data->AddHistogram(ecalHist);
   data->AddHistogram(hcalHist);
   TEveCalo3D* calo = new TEveCalo3D(data);
   calo->SetBarrelRadius(129);
   calo->SetEndCapPos(300);
   calo->SetPalette(pal);
   calo->SetTowerHeight(0.05);
   gEve->AddElement(calo);

   // projections
   TEveViewer* nv = gEve->SpawnNewViewer("NLT Projected");
   TEveScene*  ns = gEve->SpawnNewScene("Projected Event");
   nv->AddScene(ns);
   TGLViewer* v = nv->GetGLViewer();
   v->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   TGLCameraMarkupStyle* mup = v->GetCameraMarkup();
   if(mup) mup->SetShow(kFALSE);

   TEveProjectionManager* mng = new TEveProjectionManager();
   mng->SetProjection(TEveProjection::kPT_RhoZ);
   gEve->AddElement(mng, ns);
   gEve->AddToListTree(mng, kTRUE);

   mng->ImportElements(gsre);
   mng->ImportElements(calo);

   gEve->Redraw3D(1);
}

