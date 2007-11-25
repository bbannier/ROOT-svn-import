// $Id: MUON_geomAll.C,v 1.2 2007/03/15 14:39:38 mtadel Exp $

void MUON_geomAll()
{

  using namespace std;

  TString dataPath = TString(Alieve::gEvent->GetTitle());
  dataPath.Append("/geometry.root");
  gGeoManager = gEve->GetGeometry(dataPath.Data());

  Reve::GeoTopNodeRnrEl* topn_re = new Reve::GeoTopNodeRnrEl
    (gGeoManager, gGeoManager->GetTopNode());
  
  gEve->AddGlobalElement(topn_re);

  gEve->Redraw3D(kTRUE);
  
}
