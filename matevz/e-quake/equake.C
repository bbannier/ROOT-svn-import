class TQuakeViz;

TQuakeViz* equake()
{
  TEveManager::Create();

  gSystem->Load("libQuakeViz");

  gStyle->SetPalette(1, 0);
  TEveRGBAPalette* pal = new TEveRGBAPalette(0, 100, kTRUE);

  TQuakeViz* qv = new TQuakeViz;
  qv->SetPalette(pal);
  qv->SetMainColor(Color_t(kCyan - 10));
  qv->SetMainTransparency(50);

  qv->ReadData("data");
  qv->InitVizState(300);

  gEve->AddElement(qv);
  gEve->Redraw3D(kTRUE);

  gEve->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
  gEve->GetGLViewer()->SetClearColor(kWhite);
  // Names from ROOT color wheel, relative offsets as in GUI widget.
  // gEve->GetGLViewer()->SetClearColor(kCyan - 10);

  gEve->EditElement(qv);

  return qv;
}
