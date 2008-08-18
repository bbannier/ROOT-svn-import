
void track()
{
   gSystem->IgnoreSignal(kSigSegmentationViolation, true);
   TEveManager::Create();

   TEveTrackPropagator* prop = new TEveTrackPropagator();
   prop->SetMagField(-3.8);
   prop->SetEditPathMarks(kTRUE);
   // p->SetFitDaughters(1);
   prop->SetMaxR(123);
   prop->SetMaxZ(300);
   prop->RefPMAtt().SetMarkerColor(20);
   prop->RefPMAtt().SetMarkerStyle(4);
   prop->RefPMAtt().SetMarkerSize(4);

   // cms 
   // track = make_track74(prop);
   // track = make_track48(prop);
   track = make_muon(prop);
   // make_projected(track);
   gEve->Redraw3D(1);

   // make_projected(track);
}


//______________________________________________________________________________
TEveTrack* make_muon(TEveTrackPropagator* prop)
{
   prop->SetMaxR(750);
   prop->SetMaxZ(1100);

   TEveRecTrack *rc = new TEveRecTrack();
   rc->fV.Set(31.020117, 605.974976, 235.893448);
   rc->fP.Set(0.791401, 7.402456, 10.264499);
   rc->fSign = 1;
   TEveTrack* track = new TEveTrack(rc, prop);
   track->SetElementName("Moun 0");

   // first daughter
   TEvePathMark* pm = 0;
   pm = new TEvePathMark(TEvePathMark::kDaughter);
   pm->fV.Set(-1.419458e+02, -7.357000e+02, 5.291969e+02);
   track->AddPathMark(*pm);

   track->SetLineColor(4);
   track->SetRnrPoints(kTRUE);
   track->SetMarkerColor(kGreen);
   track->MakeTrack();

   gEve->AddElement(track);
   return track;
}


//______________________________________________________________________________
TEveTrack* make_track74(TEveTrackPropagator* prop)
{

   TEveRecTrack *rc = new TEveRecTrack();
   rc->fV.Set(0.010055, -0.000001, 3.379401);
   rc->fP.Set(-2.398941, -1.343142, -14.020207);
   rc->fSign = -1;
   TEveTrack* track = new TEveTrack(rc, prop);
   track->SetElementName("Track 74");

   TEvePathMark* pm = 0;
   // first daughter
   pm = new TEvePathMark(TEvePathMark::kDaughter);
   pm->fV.Set( -3.589083, -2.059227, -17.76625);
   track->AddPathMark(*pm);


   // second daughter
   pm = new TEvePathMark(TEvePathMark::kDaughter);
   pm->fV.Set(-42.55886,  -30.66510,  -264.6816);
   track->AddPathMark(*pm);

   track->SetLineColor(4);
   track->SetRnrPoints(kTRUE);
   track->SetMarkerColor(kGreen);
   track->MakeTrack();

   gEve->AddElement(track);
   return track;
}

//______________________________________________________________________________
TEveTrack* make_track48(TEveTrackPropagator* prop)
{
   TEveRecTrack *rc = new TEveRecTrack();
   rc->fV.Set(0.028558, -0.000918, 3.691919);
   rc->fP.Set(0.767095, -2.400006, -0.313103);
   rc->fSign = -1;
   TEveTrack* track = new TEveTrack(rc, prop);
   track->SetElementName("Track 48");

   // first daughter
   TEvePathMark* pm;
   pm = new TEvePathMark(TEvePathMark::kDaughter);
   pm->fV.Set(1.479084, -4.370661, 3.119761);
   track->AddPathMark(*pm);


   // second daughter
   pm = new TEvePathMark(TEvePathMark::kDaughter);
   pm->fV.Set(57.72345, -89.77011, -9.783746);
   track->AddPathMark(*pm);

   track->SetLineColor(4);
   track->SetRnrPoints(kTRUE);
   track->SetMarkerColor(kGreen);
   track->MakeTrack();
   gEve->AddElement(track);
   return track;
}

//______________________________________________________________________________
void make_projected(TEveTrack *track)
{

  TEveViewer* v1 = gEve->SpawnNewViewer("2D Viewer");
  TEveScene*  s1 = gEve->SpawnNewScene("Projected Event");
  v1->AddScene(s1);
  TGLViewer* v = v1->GetGLViewer();
  v->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
  v->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);

  TEveProjectionManager* mng = new TEveProjectionManager();
  mng->SetProjection(TEveProjection::kPT_RhoZ);
  gEve->AddElement(mng, s1);
  gEve->AddToListTree(mng, kTRUE);
  mng->ImportElements(track);

}
