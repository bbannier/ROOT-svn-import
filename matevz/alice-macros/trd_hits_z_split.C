// $Id: trd_hits_z_split.C,v 1.3 2006/10/12 14:08:45 mtadel Exp $

void trd_hits_z_split(const char *varexp    = "fX:fY:fZ:fZ",
		      const char *selection = "")
{
  AliRunLoader* rl =  Alieve::Event::AssertRunLoader();
  rl->LoadHits("TRD");

  TTree* ht = rl->GetTreeH("TRD", false);

  Reve::PointSetArray* l = new Reve::PointSetArray("TRD hits - Z Slices", "");
  l->SetMarkerColor((Color_t)7);
  l->SetMarkerStyle(20); // full circle
  l->SetMarkerSize(.5);
  
  gEve->AddElement(l);
  l->InitBins("Z", 20, -360, 360);

  TPointSelector ps(ht, l, varexp, selection);
  ps.Select();

  l->CloseBins();

  gEve->Redraw3D();
}
