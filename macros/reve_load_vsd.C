// $Header: /soft/cvsroot/AliRoot/EVE/macros/reve_load_vsd.C,v 1.1.1.1 2006/05/09 11:38:49 hristov Exp $

void reve_load_vsd(const Text_t* vsd = "AliVSD.root")
{
  gReve->GetSelector()->LoadVSD(vsd);
  gReve->GetSelector()->SelectHits();
}
