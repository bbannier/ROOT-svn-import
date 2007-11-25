// $Id: geom_t0.C,v 1.3 2007/10/22 14:49:12 mtadel Exp $

void geom_t0()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");

  Reve::ElementList* list = new Reve::ElementList("T0");
  gEve->AddGlobalElement(list);

  TGeoNode* node;
  Reve::GeoTopNodeRnrEl* re;

  node = gGeoManager->GetTopVolume()->FindNode("0STR_1");
  re = new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  re->UseNodeTrans();
  gEve->AddGlobalElement(re, list);

  node = gGeoManager->GetTopVolume()->FindNode("0STL_1");
  re = new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  re->UseNodeTrans();
  gEve->AddGlobalElement(re, list);

  gEve->Redraw3D();
}
