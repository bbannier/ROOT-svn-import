// $Id: geom_ddip.C,v 1.5 2007/03/24 18:49:32 mtadel Exp $

void geom_ddip()
{
  using namespace std;

  gGeoManager = gReve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");
  TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("DDIP_1");

  Reve::GeoTopNodeRnrEl* re = new Reve::GeoTopNodeRnrEl(gGeoManager, node);
  re->UseNodeTrans();
  gReve->AddGlobalRenderElement(re);
  gReve->Redraw3D();
}
