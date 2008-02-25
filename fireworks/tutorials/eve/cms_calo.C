#include "TEveProjections.h"

const char* histFile = "http://amraktad.web.cern.ch/amraktad/cms_calo_hist.root";

void cms_calo()
{
  TFile::SetCacheFileDir(".");
  TEveManager::Create();
  TEveScene* s = gEve->SpawnNewScene("Projected Event");
  gEve->GetDefViewer()->AddScene(s);
  TGLViewer* v = (TGLViewer *)gEve->GetGLViewer();
  v->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);

  // palette
  gStyle->SetPalette(1, 0);
  TEveRGBAPalette* pal = new TEveRGBAPalette(0, 100);
  pal->SetLimits(0, 50);
  Color_t pdf = 4;
  pal->SetDefaultColor(pdf);

  // projections
  TEveProjectionManager* mng = new TEveProjectionManager();
  mng->SetProjection(TEveProjection::kPT_RhoZ);
  gEve->AddElement(mng, s);
  gEve->AddToListTree(mng, kTRUE);

  // calo event
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

  gEve->AddElement(calo);
  mng->ImportElements(calo);

  gEve->GetSelection()->SetPickToSelect(1);
  gEve->GetHighlight()->SetPickToSelect(0);

  gEve->Redraw3D(1);
}















void marker()
{
   TEvePointSet* marker = new TEvePointSet(8);
   marker->SetName("Origin marker");
   marker->SetMarkerColor(6);
   marker->SetMarkerStyle(3);
   Float_t a = 100;
   marker->SetPoint(0, a,  +a, +a);
   marker->SetPoint(1, a,  -a, +a);
   marker->SetPoint(2, -a, -a, +a);
   marker->SetPoint(3, -a, +a, +a);
   marker->SetPoint(4, +a, +a, -a);
   marker->SetPoint(5, +a, -a, -a);
   marker->SetPoint(6, -a, +a, -a);
   marker->SetPoint(7, -a, -a, -a);
   gEve->AddElement(marker);
}


void make_calo(const char *fname, Bool_t is_ecal)
{
  TEveCalo* calo = new TEveCalo(is_ecal? "ecal":"hcal");
  calo->SetBarrelRadius(129);
  calo->SetEndCapPos(300);

  TEveRGBAPalette* pal = new TEveRGBAPalette(0, 50);
  pal->SetLimits(0, 50);
  // calo->SetPalette(pal);
  calo->Reset(1000);
  FILE* file = fopen(fname, "r");
  while (1) {
    Int_t detID;
    Float_t val, eta1, eta2, phi1, phi2;
    Int_t n = fscanf(file, 
                     "%d\t %f\t %f\t %f\t %f\t %f", 
                     &detID, &val, &phi1, &phi2, &eta1, &eta2);
    if (n < 6)
      break;

    printf("%d %f (%f %f) (%f %f) \n", detID, val, phi1, phi2, eta1, eta2);
    TEveCalo::ECellType_e ct = detID ? (TEveCalo::kEndCap) : (TEveCalo::kBarrel);
    calo->AddCell(ct, is_ecal? val : val*10, phi1, phi2, eta1, eta2);
  }
  fclose(file);
  calo->RefitPlex();
  calo->ComputeBBox();
  gEve->AddElement(calo); 
}
