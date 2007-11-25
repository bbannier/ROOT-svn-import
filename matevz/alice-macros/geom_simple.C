// $Id: geom_simple.C,v 1.3 2006/08/29 14:30:19 mtadel Exp $

void geom_simple()
{
  using namespace std;

  gGeoManager = gEve->GetGeometry("$REVESYS/alice-data/simple_geo.root");

  Reve::GeoTopNodeRnrEl* topn_re = new Reve::GeoTopNodeRnrEl
    (gGeoManager, gGeoManager->GetTopNode());
  gEve->AddGlobalElement(topn_re);
  gEve->Redraw3D();
}
