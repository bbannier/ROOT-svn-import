// Stand-Alone reve initialization.
// If class TEveManager is not know it attempts to load
// "libReve".

void reve_sa_init(int mode=1)
{
  TClass* c = gROOT->GetClass("TEveManager");
  if (!c) {
    Info("reve_sa_init", "tring to load libReve.");
    //gSystem->Load("libTreePlayer");
    //gSystem->Load("libGeomPainter");
    //gSystem->Load("libGed");
    gSystem->Load("libRGL");
    gSystem->Load("libEG");
    if (gSystem->Load("libReve") == -1)
      Warning("reve_sa_init", "loading of libReve failed.");
  }

  SetupEnvironment();
  TEveManager::SpawnGui(mode);
}
