// $Id: geom_tpc.C,v 1.3 2006/08/29 14:30:19 mtadel Exp $

void geom_tpc()
{
  using namespace std;

  gGeoManager = gReve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("TPC_M_1");

  Reve::GeoTopNodeRnrEl* tpc_re = 
    new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  gReve->AddGlobalRenderElement(tpc_re);
  gReve->Redraw3D();
}
