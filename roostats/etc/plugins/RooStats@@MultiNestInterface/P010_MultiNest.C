void P010_MultiNest()
{
   gPluginMgr->AddHandler("RooStats::MultiNestInterface", "MultiNest", "RooStats::MultiNest",
      "MultiNestPlugin", "MultiNest( )");
}
