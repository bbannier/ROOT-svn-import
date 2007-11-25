// $Id: geom_acorde.C,v 1.2 2007/10/09 07:37:54 mtadel Exp $

void geom_acorde()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("geometry.root");

  Reve::ElementList* list = new Reve::ElementList("ACORDE");
  gEve->AddGlobalElement(list);

  for(Int_t i=1; i<61; ++i) {
    char form[10000];
    sprintf(form, "ACORDE1_%d", i);
    TGeoNode* node = gGeoManager->GetTopVolume()->FindNode(form);       
    Reve::GeoTopNodeRnrEl* re =  new Reve::GeoTopNodeRnrEl(gGeoManager, node);
    re->UseNodeTrans();
    gEve->AddGlobalElement(list, re);
    // gEve->AddGlobalElement(re, list); // For EVE-dev
  }
  
  gEve->Redraw3D();  
}
