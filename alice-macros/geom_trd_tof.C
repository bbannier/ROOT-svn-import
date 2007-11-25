// $Id: geom_trd_tof.C,v 1.3 2006/10/12 14:08:45 mtadel Exp $

void geom_trd_tof()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("B077_1");

  Reve::GeoTopNodeRnrEl* its_re = 
    new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  gEve->AddGlobalElement(its_re);
  gEve->Redraw3D();
}
