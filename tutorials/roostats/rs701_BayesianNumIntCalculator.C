void rs701_BayesianNumIntCalculator()
{
  using namespace RooFit;
  using namespace RooStats;

  RooWorkspace* w = new RooWorkspace("w",true);
  w->factory("SUM::pdf(s[0,15]*Uniform(x[0,1]),b[0.001,0,2]*Uniform(x))");
  w->factory("Gaussian::prior_b(b,0.001,0.0005)");
  w->factory("PROD::model(pdf,prior_b)");
  RooAbsPdf* model = w->pdf("model");  // pdf*priorNuisance
  RooArgSet nuisanceParameters(*(w->var("b")));

  w->factory("Uniform::priorPOI(s)");
  RooAbsRealLValue* POI = w->var("s");
  RooAbsPdf* priorPOI = w->pdf("priorPOI");

  w->factory("n[1]"); // observed number of events
  RooDataSet data("data","",RooArgSet(*(w->var("x")),*(w->var("n"))),"n");
  data.add(RooArgSet(*(w->var("x"))),w->var("n")->getVal());

  BayesianNumIntCalculator bcalc("bcalc","",model,&data,POI,priorPOI,&nuisanceParameters);
  RooPlot* plot = bcalc.PlotPosterior();
  SimpleInterval* interval = bcalc.GetInterval();
  std::cout << "90% CL interval: [ " << interval.LowerLimit() << " - " << interval.UpperLimit() << " ] or 95% CL limits\n";

  // observe one event while expecting one background event -> the 95% CL upper limit on s is 4.10
}
