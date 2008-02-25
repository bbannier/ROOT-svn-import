void geom_alice_tpc()
{
   TEveManager::Create();

   gGeoManager = gEve->GetGeometry("alja1.root");

   //TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("TPC_M_1");
   TEveGeoTopNode* tpc = new TEveGeoTopNode(gGeoManager, gGeoManager->GetTopNode());
   gEve->AddGlobalElement(tpc);

   gEve->Redraw3D(kTRUE);
}
