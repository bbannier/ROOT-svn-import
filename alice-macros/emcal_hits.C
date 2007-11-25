// $Id: emcal_hits.C,v 1.2 2007/10/22 14:49:12 mtadel Exp $

Reve::PointSet*
emcal_hits(const char *varexp    = "fX:fY:fZ",
	   const char *selection = "",
	   Reve::Element* cont = 0)
{
  AliRunLoader* rl =  Alieve::Event::AssertRunLoader();
  rl->LoadHits("EMCAL");

  TTree* ht = rl->GetTreeH("EMCAL", false);
  
  //PH The line below is replaced waiting for a fix in Root
  //PH which permits to use variable siza arguments in CINT
  //PH on some platforms (alphalinuxgcc, solariscc5, etc.)
  //PH  Reve::PointSet* points = new Reve::PointSet(Form("EMCAL Hits '%s'", selection));
  char form[1000];
  sprintf(form,"EMCAL Hits '%s'", selection);
  Reve::PointSet* points = new Reve::PointSet(form);

  TPointSelector ps(ht, points, varexp, selection);
  ps.Select();

  if(points->Size() == 0 && gEve->GetKeepEmptyCont() == kFALSE) {
    Warning("emcal_hits", Form("No hits match '%s'", selection));
    delete points;
    return 0;
  }

  //PH  points->SetTitle(Form("N=%d", points->Size()));
  sprintf(form,"N=%d", points->Size());
  points->SetTitle(form);
  points->SetMarkerSize(.5);
  points->SetMarkerColor((Color_t)2);

  gEve->AddElement(points, cont);
  gEve->Redraw3D();

  return points;
}
