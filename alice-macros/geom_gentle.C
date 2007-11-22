// $Id: geom_gentle.C,v 1.2 2007/10/22 14:49:12 mtadel Exp $

Reve::GeoShapeRnrEl* geom_gentle()
{
  TFile f("$REVESYS/alice-data/gentle_geo.root");
  TGeoShapeExtract* gse = (TGeoShapeExtract*) f.Get("Gentle");
  Reve::GeoShapeRnrEl* gsre = Reve::GeoShapeRnrEl::ImportShapeExtract(gse, 0);
  f.Close();

  return gsre;
}
