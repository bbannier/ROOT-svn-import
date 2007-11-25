
Reve::NLTProjector* NLT_test(Reve::Element* top=0)
{
  using namespace Reve;

  Scene* s = gEve->SpawnNewScene("Projected Event");
  gEve->GetDefViewer()->AddScene(s);

  TGLViewer* v = (TGLViewer *)gEve->GetGLViewer();
  v->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
  TGLCameraMarkupStyle* mup = v->GetCameraMarkup();
  if(mup) mup->SetShow(kFALSE);

  NLTProjector* p = new NLTProjector;
  p->SetProjection(NLTProjection::PT_RhoZ, 0.01);

  gEve->AddToListTree(p, kTRUE);
  gEve->AddElement(p, s);

  top = gEve->GetCurrentEvent();
  if (top)
    p->ImportElements(top);

  gEve->Redraw3D(kTRUE);

  return p;
}
