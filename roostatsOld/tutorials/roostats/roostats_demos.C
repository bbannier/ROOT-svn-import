{
  // This macro shows a control bar to run the RooStats
  // tutorials.
  // To execute an item, click with the left mouse button.

  gROOT->Reset();

  //Add the tutorials directory to the macro path
  //This is necessary in case this macro is executed from another user directory
  TString dir = gSystem->UnixPathName(gInterpreter->GetCurrentMacroName());
  dir.ReplaceAll("roostats_demos.C","");
  dir.ReplaceAll("/./","");
  const char *current = gROOT->GetMacroPath();
  gROOT->SetMacroPath(Form("%s:%s",current,dir.Data()));

  const char* path2tutorial = "./"; // change using the path w.r.t. you have the macro

  const char* buttonType = "button";
   
  TControlBar *bar = new TControlBar("vertical", "RooStats Demos",50,50);
  int ic = 0;
  bar->AddButton(Form("(%i) Limit Example",++ic),
		 Form(".x %srs101_limitexample.C+",path2tutorial),
		 "Use a Confidence Interval Calculator to set a limit",buttonType);

  bar->AddButton(Form("(%i) Limit Example",++ic),
		 Form(".x %srs101b_limitexample.C+",path2tutorial),
		 "Use a Confidence Interval Calculator to set a limit",buttonType);

  bar->AddButton(Form("(%i) Limit Example",++ic),
		 Form(".x %srs101c_limitexample.C+",path2tutorial),
		 "Use a Confidence Interval Calculator to set a limit",buttonType);

  bar->AddButton(Form("(%i) Limit Example",++ic),
		 Form(".x %srs101d_limitexample.C+",path2tutorial),
		 "Use a Confidence Interval Calculator to set a limit",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) Example: Search Particle",++ic),
		 Form(".x %srs102_hypotestwithshapes.C+",path2tutorial),
		 "Search for a new particle",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) HybridCalculator",++ic),
		 Form(".x %srs201_hybridcalculator.C+",path2tutorial),
		 "Example on how to use HybridCalculator",buttonType);

  bar->AddButton(Form("(%i) HybridCalculator",++ic),
		 Form(".x %srs202_hybridcalculator.C+",path2tutorial),
		 "Example on how to use HybridCalculator",buttonType);

  bar->AddButton(Form("(%i) HybridCalculator",++ic),
		 Form(".x %srs203_HybridCalculator_with_ModelConfig.C+",path2tutorial),
		 "Example on how to use HybridCalculator",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) sPlot",++ic),
		 Form(".x %srs301_splot.C+",path2tutorial),
		 "Example of using SPlot to unfold two distributions",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401_FeldmanCousins.C+",path2tutorial),
		 " Debugging Sampling Distribution Neyman Construction",buttonType);

  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401_debuggingSamplingDist.C+",path2tutorial),
		 " Debugging Sampling Distribution Neyman Construction",buttonType);

  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401b_debuggingSamplingDist.C+",path2tutorial),
		 " Debugging Sampling Distribution Neyman Construction",buttonType);

  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401c_debuggingSamplingDist.C+",path2tutorial),
		 " Debugging Sampling Distribution Neyman Construction",buttonType);
  
  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401c_FeldmanCousins.C+",path2tutorial),
		 " Debugging Sampling Distribution Neyman Construction",buttonType);

  bar->AddButton(Form("(%i) Feldman-Cousins",++ic),
		 Form(".x %srs401e_FeldmanCousinsFig14.C+",path2tutorial),
		 " ",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) NuMuToNuE Oscillation",++ic), 
		 Form(".x %srs401d_FeldmanCousins.C+",path2tutorial),
		 "Neutrino Oscillation Example from Feldman & Cousins",buttonType);

  ic = 0;

  bar->AddButton(Form("(%i) Counting analysis",++ic), 
		 Form(".x %srs500a_PrepareWorkspace_Poisson.C+",path2tutorial),
		 "PDF model is built for a counting analysis",buttonType);

  bar->AddButton(Form("(%i) Counting analysis",++ic), 
		 Form(".x %srs500b_PrepareWorkspace_Poisson_withSystematics.C+",path2tutorial),
		 "PDF model is built for a counting analysis with systematics uncertanty",buttonType);

  bar->AddButton(Form("(%i) Counting analysis",++ic), 
		 Form(".x %srs500c_PrepareWorkspace_GaussOverFlat.C+",path2tutorial),
		 "PDF model is built for a counting analysis (signal=Gaussian, bkg=Flat)",buttonType);

  bar->AddButton(Form("(%i) Counting analysis",++ic), 
		 Form(".x %srs500d_PrepareWorkspace_GaussOverFlat_withSystematics.C+",path2tutorial),
		 "PDF model is built for a counting analysis (signal=Gaussian,bkg=Flat)"
		 " with systematic uncertanty",buttonType);
  
  ic = 0;
  bar->AddButton(Form("(%i) Upper Limit",++ic), 
		 Form(".x %srs501_ProfileLikelihoodCalculator_limit.C+",path2tutorial),
		 "Compute with ProfileLikelihoodCalculator a 95% CL upper limit",buttonType);

  bar->AddButton(Form("(%i) Upper Limit and Significance",++ic), 
		 Form(".x %srs502_ProfileLikelihoodCalculator_significance.C+",path2tutorial),
		 "Compute with ProfileLikelihoodCalculator a 68% CL interval"
		 " and the signal significance",buttonType);

  bar->AddButton(Form("(%i) Average Limit",++ic), 
		 Form(".x %srs503_ProfileLikelihoodCalculator_averageLimit.C+",path2tutorial),
		 " ",buttonType);

  bar->AddButton(Form("(%i) Average Significance",++ic), 
		 Form(".x %srs504_ProfileLikelihoodCalculator_averageSignificance.C+",path2tutorial),
		 " ",buttonType);

  bar->AddButton(Form("(%i) HybridCalculator Significance",++ic), 
		 Form(".x %srs505_HybridCalculator_significance.C+",path2tutorial),
		 " ",buttonType);

  bar->AddButton(Form("(%i) HybridCalculator Average Significance",++ic), 
		 Form(".x %srs506_HybridCalculator_averageSignificance.C+",path2tutorial),
		 " ",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) High Level Factory",++ic), 
		 Form(".x %srs601_HLFactoryexample.C+",path2tutorial),
		 "Creating a simple model using the High Level model Factory",buttonType);

  bar->AddButton(Form("(%i) High Level Factory",++ic), 
		 Form(".x %srs602_HLFactoryCombinationexample.C+",path2tutorial),
		 "Creating a combined model using the High Level model Factory",buttonType);

  bar->AddButton(Form("(%i) High Level Factory",++ic), 
		 Form(".x %srs603_HLFactoryElaborateExample.C+",path2tutorial),
		 "Creating a combined model using the High Level model Factory",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) BayesianCalculator",++ic), 
		 Form(".x %srs701_BayesianCalculator.C+",path2tutorial),
		 "Example of using the BayesianCalculator class",buttonType);

  bar->AddButton(Form("(%i) HypoTestInverter",++ic), 
		 Form(".x %srs801_HypoTestInverter.C+",path2tutorial),
		 "Example of using the HypoTestInverter class",buttonType);

  ic = 0;
  bar->AddButton(Form("(%i) Bernstein Correction",++ic), 
		 Form(".x %srs_bernsteinCorrection.C+",path2tutorial),
		 "Shows usage of a the BernsteinCorrection utility",buttonType);

  bar->AddButton(Form("(%i) Rings",++ic), 
		 Form(".x %srings.C+",path2tutorial),
		 " ",buttonType);

  bar->AddButton(Form("(%i) Combination Searches",++ic), 
		 Form(".x %srs_numberCountingCombination.C+",path2tutorial),
		 "Shows an example of a combination of"
		 " two searches using number counting with background uncertainty",buttonType);  

  bar->AddButton(Form("(%i) Standalone utilities",++ic), 
		 Form(".x %srs_numbercountingutils.C+",path2tutorial),
		 "Shows an example of the RooStats standalone utilities"
		 " that calculate the p-value or Z value",buttonType);

  //  bar->AddSeparator();
  bar->AddButton("Quit", ".q", "Quit", buttonType );

  // set the style 
  bar->SetTextColor("blue");
  bar->SetButtonWidth(300);
  bar->SetNumberOfColumns(2);

  bar->Show();

  gROOT->SaveContext();

}
