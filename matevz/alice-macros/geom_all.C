// $Id: geom_all.C,v 1.3 2006/10/12 14:08:45 mtadel Exp $

void geom_all()
{
  using namespace std;

  gGeoManager = gReve->GetGeometry("$REVESYS/alice-data/alice_fullgeo.root");

  Reve::GeoTopNodeRnrEl* topn_re = new Reve::GeoTopNodeRnrEl
    (gGeoManager, gGeoManager->GetTopNode());
  gReve->AddGlobalRenderElement(topn_re);
  gReve->Redraw3D();
}
