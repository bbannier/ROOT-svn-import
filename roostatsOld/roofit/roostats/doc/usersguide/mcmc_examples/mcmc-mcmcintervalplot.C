MCMCInterval* interval = (MCMCInterval*)mcmcCalc.GetInterval(); // must cast
MCMCIntervalPlot mcPlot(*interval);

// Draw posterior
TCanvas* c = new TCanvas("c");
mcPlot.SetLineColor(kOrange); // optional
mcPlot.SetLineWidth(2);       // optional
mcPlot.Draw();
