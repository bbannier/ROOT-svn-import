// $Id: geom_hmpid.C,v 1.2 2007/10/22 14:49:12 mtadel Exp $

void geom_hmpid()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");

  Reve::ElementList* list = new Reve::ElementList("HMPID");
  gEve->AddGlobalElement(list);

  for(Int_t i=0; i<7; ++i) {
    //PH The line below is replaced waiting for a fix in Root
    //PH which permits to use variable siza arguments in CINT
    //PH on some platforms (alphalinuxgcc, solariscc5, etc.)
    //PH TGeoNode* node = gGeoManager->GetTopVolume()->FindNode(Form("HMPID_%d", i));
    char form[1000];
    sprintf(form, "HMPID_%d", i);
    TGeoNode* node = gGeoManager->GetTopVolume()->FindNode(form);

    Reve::GeoTopNodeRnrEl* re = new Reve::GeoTopNodeRnrEl(gGeoManager, node);
    re->UseNodeTrans();
    gEve->AddGlobalElement(re, list);
  }

  gEve->Redraw3D();
}
