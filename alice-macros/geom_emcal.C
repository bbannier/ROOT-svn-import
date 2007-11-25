// $Id: geom_emcal.C,v 1.1 2007/04/11 16:13:00 mtadel Exp $

void geom_emcal()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("XEN1_1");

  Reve::GeoTopNodeRnrEl* emcal_re = 
    new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  gEve->AddGlobalElement(emcal_re);
  gEve->Redraw3D();
}
