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

  qv->ReadData("data_new");
  qv->InitVizState(300);

  gEve->AddElement(qv);
  gEve->Redraw3D(kTRUE);

  gEve->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
  gEve->GetGLViewer()->SetClearColor(kWhite);
  // Names from ROOT color wheel, relative offsets as in GUI widget.
  // gEve->GetGLViewer()->SetClearColor(kCyan - 10);

  gEve->EditElement(qv);

  TEveLine* line = read_midline();
  line->SetRnrSelf(kFALSE);

  return qv;
}

TEveLine*
read_midline(const char* file      = "mid_line",
             Int_t       min_n_fit = 2,
             Float_t     depth_off = 49.5)
{
  FILE* fp = fopen(file, "r");
  if (!fp) {
    Warning("read_midline", "Can't open file '%s'.", file);
    return;
  }

  TEveLine* line = new TEveLine("MidLine");
  line->SetMainColor(kBlack);

  Int_t   depth_idx, n_fit;
  Float_t x, y;

  while (fscanf(fp, "%d %d %f %f", &depth_idx, &n_fit, &x, &y) == 4)
  {
    if (n_fit >= min_n_fit)
    {
      line->SetNextPoint(0.785534 * x, 1.111329444 * y,
                         -(depth_off + depth_idx));
    }
  }

  fclose(fp);

  gEve->AddElement(line);
  gEve->Redraw3D();

  return line;
}
