// $Id: its_hits_layer_split.C,v 1.1 2007/04/20 09:27:40 mtadel Exp $

void its_hits_layer_split(const char *varexp    = "fX:fY:fZ:fLayer",
                          const char *selection = "")
{
  // Extracts 'major' TPC hits (not the compressed ones).
  // This gives ~2.5% of all hits.

  AliRunLoader* rl =  Alieve::Event::AssertRunLoader();
  rl->LoadHits("ITS");

  TTree* ht = rl->GetTreeH("ITS", false);

  Reve::PointSetArray* l = new Reve::PointSetArray("ITS hits - Layer Slices", "");
  l->SetMarkerColor((Color_t)2);
  l->SetMarkerStyle(2); // cross
  l->SetMarkerSize(.2);

  gEve->AddElement(l);
  l->InitBins("Layer", 6, 0.5, 6.5);

  TPointSelector ps(ht, l, varexp, selection);
  ps.Select();

  l->CloseBins();

  gEve->Redraw3D();
}
