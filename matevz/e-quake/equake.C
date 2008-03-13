class TQuakeViz;

TQuakeViz* equake()
{
  TEveManager::Create();

  gSystem->Load("libQuakeViz");

  TQuakeViz* qv = new TQuakeViz;
  qv->ReadData("data");

  qv->SetMainColor(Color_t(kCyan - 10));
  qv->SetMainTransparency(50);

  gEve->AddElement(qv);
  gEve->Redraw3D(kTRUE);

  gEve->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOY);

  return qv;
}
