// $Id: geom_its.C,v 1.3 2006/08/29 14:30:19 mtadel Exp $

void geom_its()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("ITSV_1");

  Reve::GeoTopNodeRnrEl* its_re = 
    new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  gEve->AddGlobalElement(its_re);
  gEve->Redraw3D();
}
