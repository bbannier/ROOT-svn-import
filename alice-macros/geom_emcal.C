// $Id: geom_emcal.C,v 1.1 2007/04/11 16:13:00 mtadel Exp $

void geom_emcal()
{
  using namespace std;

  gGeoManager = gReve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("XEN1_1");

  Reve::GeoTopNodeRnrEl* emcal_re = 
    new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  gReve->AddGlobalRenderElement(emcal_re);
  gReve->Redraw3D();
}
