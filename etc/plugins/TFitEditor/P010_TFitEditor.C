void P010_TFitEditor()
{
   gPluginMgr->AddHandler("TFitEditor", "*", "TFitEditor",
      "FitPanel", "PluginHandler(TVirtualPad*, TObject*)");
}
