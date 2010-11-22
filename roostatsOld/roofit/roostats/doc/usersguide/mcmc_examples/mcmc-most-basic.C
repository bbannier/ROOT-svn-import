// data is a RooAbsData, model is a RooAbsPdf,
// and parametersOfInterest is a RooArgSet
MCMCCalculator mc(data, model, parametersOfInterest);
ConfInterval* mcmcInterval = mc.GetInterval();
