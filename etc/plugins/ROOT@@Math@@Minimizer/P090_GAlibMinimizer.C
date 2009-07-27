void P090_GAlibMinimizer()
{
   gPluginMgr->AddHandler("ROOT::Math::Minimizer", "GAlibMin", "ROOT::Math::GAlibMinimizer",
      "GAlibMin", "GAlibMinimizer(int)");
}
