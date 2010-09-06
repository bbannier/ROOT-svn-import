#ifndef ROOT_TEfficiency_cxx
#define ROOT_TEfficiency_cxx

//standard header
#include <vector>
#include <string>
#include <cmath>

//ROOT headers
#include "Math/QuantFuncMathCore.h"
#include "TBinomialEfficiencyFitter.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TList.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TVirtualPad.h"

//custom headers
#include "TEfficiency.h"

//default values
const Double_t kDefBetaAlpha = 1;
const Double_t kDefBetaBeta = 1;
const Double_t kDefConfLevel = 0.95;
const Int_t kDefStatOpt = TEfficiency::kFCP;
const Double_t kDefWeight = 1;

ClassImp(TEfficiency)

//______________________________________________________________________________
/* Begin_Html <center><h2>TEfficiency - a class to handle efficiencies
histograms</h2></center> End_Html */

//______________________________________________________________________________
TEfficiency::TEfficiency():
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fLowerBound(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fPassedHistogram(0),
   fTotalHistogram(0),
   fWeight(kDefWeight)
{
   //default constructor
   //
   //should not be used explicitly
   
   SetStatisticOption(kDefStatOpt);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const TH1& passed,const TH1& total):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //constructor using two existing histograms as input
   //
   //Input: passed - contains the events fullfilling some criteria
   //       total  - contains all investigated events
   //
   //Notes: - both histograms have to have the same binning
   //       - dimension of the resulating efficiency object depends
   //         on the dimension of the given histograms
   //       - Clones of both histograms are stored internally
   //       - The function SetName(total.GetName() + "_clone") is called to set
   //         the names of the new object and the internal histograms..
   //       - The created TEfficiency object is NOT appended to a directory. It
   //         will not be written to disk during the next TFile::Write() command
   //         in order to prevent duplication of data. If you want to save this
   //         TEfficiency object anyway, you can either append it to a
   //         directory by calling SetDirectory(TDirectory*) or write it
   //         explicitly to disk by calling Write().

   //check consistency of histograms
   if(CheckConsistency(passed,total)) {
       Bool_t bStatus = TH1::AddDirectoryStatus();
       TH1::AddDirectory(kFALSE);
       fTotalHistogram = (TH1*)total.Clone();
       fPassedHistogram = (TH1*)passed.Clone();
       TH1::AddDirectory(bStatus);

       char buffer[100];
       strcpy(buffer,total.GetName());
       strcat(buffer,"_clone");
       SetName(buffer);
   }
   else {
      Error("TEfficiency(const TH1&,const TH1&)","histograms are not consistent -> results are useless");
      Warning("TEfficiency(const TH1&,const TH1&)","using two empty TH1D('h1','h1',10,0,10)");

      Bool_t bStatus = TH1::AddDirectoryStatus();
      TH1::AddDirectory(kFALSE);
      fTotalHistogram = new TH1D("h1_total","h1 (total)",10,0,10);
      fPassedHistogram = new TH1D("h1_passed","h1 (passed)",10,0,10);
      TH1::AddDirectory(bStatus);
   }   

   SetStatisticOption(kDefStatOpt);
   SetDirectory(0);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbins,
				const Double_t* xbins):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 1-dimensional TEfficiency object with variable bin size
   //
   //constructor creates two new and empty histograms with a given binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbins  - number of bins on the x-axis
   //        xbins  - array of length (nbins + 1) with low-edges for each bin
   //                 xbins[nbinsx] ... lower edge for overflow bin
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH1D("total","total",nbins,xbins);
   fPassedHistogram = new TH1D("passed","passed",nbins,xbins);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbinsx,
				Double_t xlow,Double_t xup):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 1-dimensional TEfficiency object with fixed bins isze
   //
   //constructor creates two new and empty histograms with a fixed binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbinsx - number of bins on the x-axis
   //        xlow   - lower edge of first bin
   //        xup    - upper edge of last bin
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH1D("total","total",nbinsx,xlow,xup);
   fPassedHistogram = new TH1D("passed","passed",nbinsx,xlow,xup);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbinsx,
			   Double_t xlow,Double_t xup,Int_t nbinsy,
			   Double_t ylow,Double_t yup):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 2-dimensional TEfficiency object with fixed bin size
   //
   //constructor creates two new and empty histograms with a fixed binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbinsx - number of bins on the x-axis
   //        xlow   - lower edge of first x-bin
   //        xup    - upper edge of last x-bin
   //        nbinsy - number of bins on the y-axis
   //        ylow   - lower edge of first y-bin
   //        yup    - upper edge of last y-bin
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH2D("total","total",nbinsx,xlow,xup,nbinsy,ylow,yup);
   fPassedHistogram = new TH2D("passed","passed",nbinsx,xlow,xup,nbinsy,ylow,yup);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbinsx,
			   const Double_t* xbins,Int_t nbinsy,
			   const Double_t* ybins):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 2-dimensional TEfficiency object with variable bin size
   //
   //constructor creates two new and empty histograms with a given binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbinsx - number of bins on the x-axis
   //        xbins  - array of length (nbins + 1) with low-edges for each bin
   //                 xbins[nbinsx] ... lower edge for overflow x-bin
   //        nbinsy - number of bins on the y-axis
   //        ybins  - array of length (nbins + 1) with low-edges for each bin
   //                 ybins[nbinsy] ... lower edge for overflow y-bin
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH2D("total","total",nbinsx,xbins,nbinsy,ybins);
   fPassedHistogram = new TH2D("passed","passed",nbinsx,xbins,nbinsy,ybins);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbinsx,
			   Double_t xlow,Double_t xup,Int_t nbinsy,
			   Double_t ylow,Double_t yup,Int_t nbinsz,
			   Double_t zlow,Double_t zup):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 3-dimensional TEfficiency object with fixed bin size
   //
   //constructor creates two new and empty histograms with a fixed binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbinsx - number of bins on the x-axis
   //        xlow   - lower edge of first x-bin
   //        xup    - upper edge of last x-bin
   //        nbinsy - number of bins on the y-axis
   //        ylow   - lower edge of first y-bin
   //        yup    - upper edge of last y-bin
   //        nbinsz - number of bins on the z-axis
   //        zlow   - lower edge of first z-bin
   //        zup    - upper edge of last z-bin
   //        
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH3D("total","total",nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup);
   fPassedHistogram = new TH3D("passed","passed",nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const char* name,const char* title,Int_t nbinsx,
			   const Double_t* xbins,Int_t nbinsy,
			   const Double_t* ybins,Int_t nbinsz,
			   const Double_t* zbins):
   fBeta_alpha(kDefBetaAlpha),
   fBeta_beta(kDefBetaBeta),
   fConfLevel(kDefConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(kDefWeight)
{
   //create 3-dimensional TEfficiency object with variable bin size
   //
   //constructor creates two new and empty histograms with a given binning
   //
   // Input: name   - the common part of the name for both histograms(no blanks)
   //                 fTotalHistogram has name: name + "_total"
   //                 fPassedHistogram has name: name + "_passed"
   //        title  - the common part of the title for both histogram
   //                 fTotalHistogram has title: title + " (total)"
   //                 fPassedHistogram has title: title + " (passed)"
   //        nbinsx - number of bins on the x-axis
   //        xbins  - array of length (nbins + 1) with low-edges for each bin
   //                 xbins[nbinsx] ... lower edge for overflow x-bin
   //        nbinsy - number of bins on the y-axis
   //        ybins  - array of length (nbins + 1) with low-edges for each bin
   //                 xbins[nbinsx] ... lower edge for overflow y-bin
   //        nbinsz - number of bins on the z-axis
   //        zbins  - array of length (nbins + 1) with low-edges for each bin
   //                 xbins[nbinsx] ... lower edge for overflow z-bin
   //
   // Note: The extra token " (total)" or " (passed)" will be inserted in front
   //       of the first semicolon in title (or at the end if title does not
   //       contain any semicolon). Therefore it is possible to label the axis
   //       as usual.

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = new TH3D("total","total",nbinsx,xbins,nbinsy,ybins,nbinsz,zbins);
   fPassedHistogram = new TH3D("passed","passed",nbinsx,xbins,nbinsy,ybins,nbinsz,zbins);
   TH1::AddDirectory(bStatus);

   Build(name,title);
}

//______________________________________________________________________________
TEfficiency::TEfficiency(const TEfficiency& rEff):
   TNamed(),
   TAttLine(),
   TAttFill(),
   TAttMarker(),
   fBeta_alpha(rEff.fBeta_alpha),
   fBeta_beta(rEff.fBeta_beta),
   fConfLevel(rEff.fConfLevel),
   fDirectory(0),
   fFunctions(0),
   fPaintGraph(0),
   fPaintHisto(0),
   fWeight(rEff.fWeight)
{
   //copy constructor
   //
   //The list of associated objects (e.g. fitted functions) is not copied.
   //
   //Note: - SetName(rEff.GetName() + "_copy") is called to set the names of the
   //        object and the histograms.
   //      - The titles are set by calling SetTitle("[copy] " + rEff.GetTitle()).

   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   fTotalHistogram = (TH1*)((rEff.fTotalHistogram)->Clone());
   fPassedHistogram = (TH1*)((rEff.fPassedHistogram)->Clone());
   TH1::AddDirectory(bStatus);

   char buffer[100];
   strcpy(buffer,rEff.GetName());
   strcat(buffer,"_copy");
   SetName(buffer);
   strcpy(buffer,"[copy] ");
   strcat(buffer,rEff.GetTitle());
   SetTitle(buffer);
   
   SetStatisticOption(rEff.GetStatisticOption());

   SetDirectory(rEff.GetDirectory());

   //copy style
   SetLineColor(rEff.GetLineColor());
   SetLineStyle(rEff.GetLineStyle());
   SetLineWidth(rEff.GetLineWidth());
   SetFillColor(rEff.GetFillColor());
   SetFillStyle(rEff.GetFillStyle());
   SetMarkerColor(rEff.GetMarkerColor());
   SetMarkerSize(rEff.GetMarkerSize());
   SetMarkerStyle(rEff.GetMarkerStyle());   
}

//______________________________________________________________________________
TEfficiency::~TEfficiency()
{
   //default destructor

   //delete all function in fFunctions
   if(fFunctions) {
      
      TIter next(fFunctions);
      TObject* obj = 0;
      while((obj = next())) {
	 delete obj;
      }

      fFunctions->Delete();
   }

   if(fDirectory)
      fDirectory->Remove(this);
   
   delete fFunctions;
   delete fTotalHistogram;
   delete fPassedHistogram;
   delete fPaintGraph;
   delete fPaintHisto;
}

//______________________________________________________________________________
Double_t TEfficiency::AgrestiCoullLow(Int_t total,Int_t passed,Double_t level)
{
   //calculates the lower boundary for the frequentist Agresti-Coull interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //Begin_Latex(separator='=',align='rl')
   // #alpha = 1 - #frac{level}{2}
   // #kappa = #Phi^{-1}(1 - #alpha,1) ... normal quantile function
   // mode = #frac{passed + #frac{#kappa^{2}}{2}}{total + #kappa^{2}}
   // #Delta = #kappa * #sqrt{#frac{mode * (1 - mode)}{total + #kappa^{2}}}
   // return =  max(0,mode - #Delta)
   //End_Latex
   
   Double_t alpha = (1.0 - level)/2;
   Double_t kappa = ROOT::Math::normal_quantile(1 - alpha,1);

   Double_t mode = (passed + 0.5 * kappa * kappa) / (total + kappa * kappa);
   Double_t delta = kappa * std::sqrt(mode * (1 - mode) / (total + kappa * kappa));
	 
   return ((mode - delta) < 0) ? 0.0 : (mode - delta);
}

//______________________________________________________________________________
Double_t TEfficiency::AgrestiCoullUp(Int_t total,Int_t passed,Double_t level)
{
   //calculates the upper boundary for the frequentist Agresti-Coull interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //Begin_Latex(separator='=',align='rl')
   // #alpha = 1 - #frac{level}{2}
   // #kappa = #Phi^{-1}(1 - #alpha,1) ... normal quantile function
   // mode = #frac{passed + #frac{#kappa^{2}}{2}}{total + #kappa^{2}}
   // #Delta = #kappa * #sqrt{#frac{mode * (1 - mode)}{total + #kappa^{2}}}
   // return =  min(1,mode + #Delta)
   //End_Latex
   
   Double_t alpha = (1.0 - level)/2;
   Double_t kappa = ROOT::Math::normal_quantile(1 - alpha,1);

   Double_t mode = (passed + 0.5 * kappa * kappa) / (total + kappa * kappa);
   Double_t delta = kappa * std::sqrt(mode * (1 - mode) / (total + kappa * kappa));
	 
   return ((mode + delta) > 1) ? 1.0 : (mode + delta);
}

//______________________________________________________________________________
Double_t TEfficiency::BayesianLow(Int_t total,Int_t passed,Double_t level,Double_t alpha,Double_t beta)
{
   //calculates the lower boundary for a baysian confidence interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //       - alpha : shape parameter > 0 for the prior distribution (fBeta_alpha)
   //       - beta  : shape parameter > 0 for the prior distribution (fBeta_beta)
   //
   //Note: The equal-tailed confidence interval is calculated which might be not
   //      the shortest interval containing the desired coverage probability.
   //
   //Calculation:
   //
   //The posterior probability in bayesian statistics is given by:
   //Begin_Latex P(#varepsilon |k,N) #propto L(#varepsilon|k,N) #times Prior(#varepsilon)End_Latex
   //As an efficiency can be interpreted as probability of a positive outcome of
   //a Bernoullli trial the likelihood function is given by the binomial
   //distribution:
   //Begin_Latex L(#varepsilon|k,N) = Binomial(N,k) #varepsilon ^{k} (1 - #varepsilon)^{N-k}End_Latex
   //At the moment only beta distributions are supported as prior probabilities
   //of the efficiency (Begin_Latex #scale[0.8]{B(#alpha,#beta)}End_Latex is the beta function):
   //Begin_Latex Prior(#varepsilon) = #frac{1}{B(#alpha,#beta)} #varepsilon ^{#alpha - 1} (1 - #varepsilon)^{#beta - 1}End_Latex
   //The posterior probability is therefore again given by a beta distribution:
   //Begin_Latex P(#varepsilon |k,N) #propto #varepsilon ^{k + #alpha - 1} (1 - #varepsilon)^{N - k + #beta - 1} End_Latex
   //The lower boundary for the equal-tailed confidence interval is given by the
   //inverse cumulative (= quantile) function for the quantile Begin_Latex #frac{1 - level}{2} End_Latex.
   //Hence it is the solution Begin_Latex #varepsilon End_Latex of the following equation:
   //Begin_Latex I_{#varepsilon}(k + #alpha,N - k + #beta) = #frac{1}{norm} #int_{0}^{#varepsilon} dt t^{k + #alpha - 1} (1 - t)^{N - k + #beta - 1} =  #frac{1 - level}{2} End_Latex

   if((alpha > 0) && (beta > 0))
      return (passed == 0)? 0.0 : ROOT::Math::beta_quantile((1-level)/2,passed+alpha,total-passed+beta);
   else
      return 0;
}

//______________________________________________________________________________
Double_t TEfficiency::BayesianUp(Int_t total,Int_t passed,Double_t level,Double_t alpha,Double_t beta)
{
   //calculates the upper boundary for a baysian confidence interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //       - alpha : shape parameter > 0 for the prior distribution (fBeta_alpha)
   //       - beta  : shape parameter > 0 for the prior distribution (fBeta_beta)
   //
   //Note: The equal-tailed confidence interval is calculated which might be not
   //      the shortest interval containing the desired coverage probability.
   //
   //Calculation:
   //
   //The posterior probability in bayesian statistics is given by:
   //Begin_Latex P(#varepsilon |k,N) #propto L(#varepsilon|k,N) #times Prior(#varepsilon)End_Latex
   //As an efficiency can be interpreted as probability of a positive outcome of
   //a Bernoullli trial the likelihood function is given by the binomial
   //distribution:
   //Begin_Latex L(#varepsilon|k,N) = Binomial(N,k) #varepsilon ^{k} (1 - #varepsilon)^{N-k}End_Latex
   //At the moment only beta distributions are supported as prior probabilities
   //of the efficiency (Begin_Latex #scale[0.8]{B(#alpha,#beta)}End_Latex is the beta function):
   //Begin_Latex Prior(#varepsilon) = #frac{1}{B(#alpha,#beta)} #varepsilon ^{#alpha - 1} (1 - #varepsilon)^{#beta - 1}End_Latex
   //The posterior probability is therefore again given by a beta distribution:
   //Begin_Latex P(#varepsilon |k,N) #propto #varepsilon ^{k + #alpha - 1} (1 - #varepsilon)^{N - k + #beta - 1} End_Latex
   //The upper boundary for the equal-tailed confidence interval is given by the
   //inverse cumulative (= quantile) function for the quantile Begin_Latex #frac{1 + level}{2} End_Latex.
   //Hence it is the solution Begin_Latex #varepsilon End_Latex of the following equation:
   //Begin_Latex I_{#varepsilon}(k + #alpha,N - k + #beta) = #frac{1}{norm} #int_{0}^{#varepsilon} dt t^{k + #alpha - 1} (1 - t)^{N - k + #beta - 1} =  #frac{1 + level}{2} End_Latex

   if((alpha > 0) && (beta > 0))
      return (passed == total)? 1.0 : ROOT::Math::beta_quantile((1+level)/2,passed+alpha,total-passed+beta);
   else
      return 1;
}

//______________________________________________________________________________
void TEfficiency::Build(const char* name,const char* title)
{
   //building standard data structure of a TEfficiency object
   //
   //Notes: - calls: SetName(name), SetTitle(title)
   //       - set the statistic option to the default (kFCP)
   //       - appends this object to the current directory
   //         SetDirectory(gDirectory)
   
   SetName(name);
   SetTitle(title);

   SetStatisticOption(kDefStatOpt);
   SetDirectory(gDirectory);
}

//______________________________________________________________________________
Bool_t TEfficiency::CheckBinning(const TH1& pass,const TH1& total)
{
   //checks binning for each axis
   //
   //It is assumed that the passed histograms have the same dimension.
   
   TAxis* ax1 = 0;
   TAxis* ax2 = 0;
   
   //check binning along x axis
   for(Int_t j = 0; j < pass.GetDimension(); ++j) {
      switch(j) {
      case 0:
	 ax1 = pass.GetXaxis();
	 ax2 = total.GetXaxis();
	 break;
      case 1:
	 ax1 = pass.GetYaxis();
	 ax2 = total.GetYaxis();
	 break;
      case 2:
	 ax1 = pass.GetZaxis();
	 ax2 = total.GetZaxis();
	 break;
      }
      
      if(ax1->GetNbins() != ax2->GetNbins())
	 return false;
      if(ax1->GetXmin() != ax2->GetXmin())
	 return false;
      if(ax1->GetXmax() != ax2->GetXmax())
	 return false;

      for(Int_t i = 1; i <= ax1->GetNbins(); ++i)
	 if(ax1->GetBinLowEdge(i) != ax2->GetBinLowEdge(i))
	    return false;
   }

   return true;
}

//______________________________________________________________________________
Bool_t TEfficiency::CheckConsistency(const TH1& pass,const TH1& total)
{
   //checks the consistence of the given histograms
   //
   //The histograms are considered as consistent if:
   //- both have the same dimension
   //- both have the same binning
   //- pass.GetBinContent(i) <= total.GetBinContent(i) for each bin i
   
   if(pass.GetDimension() != total.GetDimension())
      return false;

   if(!CheckBinning(pass,total))
      return false;

   if(!CheckEntries(pass,total))
      return false;

   return true;
}

//______________________________________________________________________________
Bool_t TEfficiency::CheckEntries(const TH1& pass,const TH1& total)
{
   //checks whether bin contents are compatible with binomial statistics
   //
   //The following inequality has to be valid for each bin i:
   // total.GetBinContent(i) >= pass.GetBinContent(i)
   //
   //and the histogram have to be filled with unit weights.
   //
   //Note: - It is assumed that both histograms have the same dimension and
   //        binning.

   //check for unit weights
   Double_t statpass[10];
   Double_t stattotal[10];

   pass.GetStats(statpass);
   total.GetStats(stattotal);

   //require: sum of weights == sum of weights^2
   if((TMath::Abs(statpass[0]-statpass[1]) > 1e-5) ||
      (TMath::Abs(stattotal[0]-stattotal[1]) > 1e-5))
      return false;

   //check: pass <= total
   Int_t nbinsx, nbinsy, nbinsz, nbins;

   nbinsx = pass.GetNbinsX();
   nbinsy = pass.GetNbinsY();
   nbinsz = pass.GetNbinsZ();

   switch(pass.GetDimension()) {
   case 1: nbins = nbinsx + 2; break;
   case 2: nbins = (nbinsx + 2) * (nbinsy + 2); break;
   case 3: nbins = (nbinsx + 2) * (nbinsy + 2) * (nbinsz + 2); break;
   default: nbins = 0;
   }
   
   for(Int_t i = 0; i < nbins; ++i) {
      if(pass.GetBinContent(i) > total.GetBinContent(i))
	 return false;
   }

   return true;
}

//______________________________________________________________________________
Double_t TEfficiency::ClopperPearsonLow(Int_t total,Int_t passed,Double_t level)
{
   //calculates the lower boundary for the frequentist Clopper-Pearson interval
   //
   //This interval is recommended by the PDG.
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //
   //The lower boundary of the Clopper-Pearson interval is the "exact" inversion
   //of the test:
   //Begin_Latex(separator='=',aling='rl')
   //P(x #geq passed; total) = #frac{1 - level}{2}
   //P(x #geq passed; total) = 1 - P(x #leq passed - 1; total)
   // = 1 - #frac{1}{norm} * #int_{0}^{1 - #varepsilon} t^{total - passed} (1 - t)^{passed - 1} dt
   // = 1 - #frac{1}{norm} * #int_{#varepsilon}^{1} t^{passed - 1} (1 - t)^{total - passed} dt
   // = #frac{1}{norm} * #int_{0}^{#varepsilon} t^{passed - 1} (1 - t)^{total - passed} dt
   // = I_{#varepsilon}(passed,total - passed + 1)
   //End_Latex
   //The lower boundary is therfore given by the Begin_Latex #frac{1 - level}{2}End_Latex quantile
   //of the beta distribution.
   //
   //Note: The connection between the binomial distribution and the regularized
   //      incomplete beta function Begin_Latex I_{#varepsilon}(#alpha,#beta)End_Latex has been used.
   
   Double_t alpha = (1.0 - level) / 2;
   return ((passed == 0) ? 0.0 : ROOT::Math::beta_quantile(alpha,passed,total-passed+1.0));
}

//______________________________________________________________________________
Double_t TEfficiency::ClopperPearsonUp(Int_t total,Int_t passed,Double_t level)
{
   //calculates the upper boundary for the frequentist Clopper-Pearson interval
   //
   //This interval is recommended by the PDG.
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //
   //The upper boundary of the Clopper-Pearson interval is the "exact" inversion
   //of the test:
   //Begin_Latex(separator='=',aling='rl')
   //P(x #leq passed; total) = #frac{1 - level}{2}
   //P(x #leq passed; total) = #frac{1}{norm} * #int_{0}^{1 - #varepsilon} t^{total - passed - 1} (1 - t)^{passed} dt
   // = #frac{1}{norm} * #int_{#varepsilon}^{1} t^{passed} (1 - t)^{total - passed - 1} dt
   // = 1 - #frac{1}{norm} * #int_{0}^{#varepsilon} t^{passed} (1 - t)^{total - passed - 1} dt
   // #Rightarrow 1 - #frac{1 - level}{2} = #frac{1}{norm} * #int_{0}^{#varepsilon} t^{passed} (1 - t)^{total - passed -1} dt
   // #frac{1 + level}{2} = I_{#varepsilon}(passed + 1,total - passed)
   //End_Latex
   //The upper boundary is therfore given by the Begin_Latex #frac{1 + level}{2}End_Latex quantile
   //of the beta distribution.
   //
   //Note: The connection between the binomial distribution and the regularized
   //      incomplete beta function Begin_Latex I_{#varepsilon}(#alpha,#beta)End_Latex has been used.
   
   Double_t alpha = (1.0 - level) / 2;
   return ((passed == total) ? 1.0 : ROOT::Math::beta_quantile(1 - alpha,passed + 1,total-passed));
}

//______________________________________________________________________________
TGraphAsymmErrors* TEfficiency::Combine(TCollection* pList,Option_t* option,
					 Int_t n,Double_t* p)
{
   //
   //options:
   // -s        : strict combining; only TEfficiency objects with the same beta
   //             prior and the flag kIsBayesian == true are combined
   // -v        : verbose mode; print information about combining
   // -cl=0.xxx : set confidence level (0 < cl < 1)
   
   TString opt = option;
   opt.ToLower();

   //parameter of prior distribution, confidence level and normalisation factor
   Double_t alpha = 0;
   Double_t beta = 0;
   Double_t level = 0;
   
   //flags for combining
   Bool_t bStrict = false;
   Bool_t bOutput = false;
   Bool_t bWeights = false;
   //list of all information needed to weight and combine efficiencies
   std::vector<TH1*> vTotal;
   std::vector<TH1*> vPassed;
   std::vector<Double_t> vWeights;
   std::vector<Double_t> vAlpha;
   std::vector<Double_t> vBeta;

   if(opt.Contains("s"))
      bStrict = true;

   if(opt.Contains("v"))
      bOutput = true;

   if(opt.Contains("cl=")) {
      sscanf(strstr(opt.Data(),"cl="),"cl=%lf",&level);
      if((level <= 0) && (level >= 1))
	 level = 0;
   }

   //are weights explicitly given
   if(n && p) {
      bWeights = true;
      for(UInt_t k = 0; k < n; ++k) {
	 if(p[k] > 0)
	    vWeights.push_back(p[k]);
	 else {
	    gROOT->Error("TEfficiency::Combine","invalid custom weight found w = %.2lf",p[k]);
	    gROOT->Info("TEfficiency::Combine","stop combining");
	    return 0;
	 }
      }
   }
   
   TIter next(pList);
   TObject* obj = 0;
   TEfficiency* pEff = 0;
   while((obj = next())) {
      pEff = dynamic_cast<TEfficiency*>(obj);
      //is object a TEfficiency object?
      if(pEff) {
	 if(!level) level = pEff->GetConfidenceLevel();
	 
	 //if strict combining, check priors, confidence level and statistic
	 if(bStrict) {
	    if(!alpha) alpha = pEff->GetBetaAlpha();
	    if(!beta) beta = pEff->GetBetaBeta();
	 
	    if(alpha != pEff->GetBetaAlpha())
	       continue;	    
	    if(beta != pEff->GetBetaBeta())
	       continue;
	    if(!pEff->UsesBayesianStat())
	       continue;
	 }
	
	 vTotal.push_back(pEff->fTotalHistogram);
	 vPassed.push_back(pEff->fPassedHistogram);

	 //no weights given -> use weights of TEfficiency objects
	 if(!bWeights)
	    vWeights.push_back(pEff->fWeight);

	 //strict combining -> using global prior
	 if(bStrict) {
	    vAlpha.push_back(alpha);
	    vBeta.push_back(beta);
	 }
	 else {
	    vAlpha.push_back(pEff->GetBetaAlpha());
	    vBeta.push_back(pEff->GetBetaBeta());
	 }
      }
   }

   //no TEfficiency objects found
   if(vTotal.empty()) {
      gROOT->Error("TEfficiency::Combine","no TEfficiency objects in given list");
      gROOT->Info("TEfficiency::Combine","stop combining");
      return 0;
   }

   //invalid number of custom weights
   if(bWeights && (n != vTotal.size())) {
      gROOT->Error("TEfficiency::Combine","number of weights n=%i differs from number of TEfficiency objects k=%i which should be combined",n,vTotal.size());
      gROOT->Info("TEfficiency::Combine","stop combining");
      return 0;
   }

   Int_t nbins_max = vTotal.at(0)->GetNbinsX();
   //check consistency of all histograms
   for(UInt_t i=0; i<vTotal.size(); ++i) {
      try {
	 TEfficiency::CheckConsistency(*vTotal.at(0),*vTotal.at(i));
      }
      catch(std::exception&) {
	 gROOT->Warning("TEfficiency::Combine","histograms are not consistent -> results may be useless");
      }
      if(vTotal.at(i)->GetNbinsX() < nbins_max) nbins_max = vTotal.at(i)->GetNbinsX();
   }

   //display information about combining
   if(bOutput) {
      gROOT->Info("TEfficiency::Combine","combining %i TEfficiency objects",vTotal.size());
      if(bWeights)
	 gROOT->Info("TEfficiency::Combine","using custom weights");
      if(bStrict) {
	 gROOT->Info("TEfficiency::Combine","using the following prior probability for the efficiency: P(e) ~ Beta(e,%.3lf,%.3lf)",alpha,beta);
      }
      else
	 gROOT->Info("TEfficiency::Combine","using individual priors of each TEfficiency object");
      gROOT->Info("TEfficiency::Combine","confidence level = %.2lf",level);
   }

   //create TGraphAsymmErrors with efficiency
   Double_t* x = new Double_t[nbins_max];
   Double_t* xlow = new Double_t[nbins_max];
   Double_t* xhigh = new Double_t[nbins_max];
   Double_t* eff = new Double_t[nbins_max];
   Double_t* efflow = new Double_t[nbins_max];
   Double_t* effhigh = new Double_t[nbins_max];

   TString formula;
   char* sub = 0;
   TF1* pdf = 0;
   Double_t w = 0;
   Double_t totalweight = 0;

   //loop over all bins
   for(Int_t i=1; i <= nbins_max; ++i) {
      //the binning of the x-axis is taken from the first total histogram
      x[i-1] = vTotal.at(0)->GetBinCenter(i);
      xlow[i-1] = x[i-1] - vTotal.at(0)->GetBinLowEdge(i);
      xhigh[i-1] = vTotal.at(0)->GetBinWidth(i) - xlow[i-1];
      //reset normalisation factor
      totalweight = 0;
      //create formula string:
      //(\sum_i weight_i * Beta(x;passed_i + alpha,total_i - passed_i + beta) )/totalweight
      formula.Clear();
      formula = "( 0 ";
      //add the sum
      for(UInt_t j=0; j < vTotal.size(); ++j) {
	 //use custom weights
	 if(bWeights)
	    w = vWeights.at(j);
	 //use generic weight = weight * N_total
	 else
	    w = vWeights.at(j)*(vTotal.at(j)->GetBinContent(i));
	 
	 totalweight += w;
	 
	 sub = new char[100];
	 sprintf(sub,"+ %lf * TMath::BetaDist(x,%lf,%lf) ",w,
		 vPassed.at(j)->GetBinContent(i)+vAlpha.at(j),vTotal.at(j)->GetBinContent(i)-
		 vPassed.at(j)->GetBinContent(i)+vBeta.at(j));
	 formula.Append(sub);
	 delete sub;
	 sub = 0;
      }
      //divide by total weight
      sub = new char[20];
      sprintf(sub,")/%lf",totalweight);
      formula.Append(sub);
      delete sub;
      sub = 0;
      
      pdf = new TF1("pdf",formula.Data(),0,1);

      //fill efficiency and errors
      Double_t q[2];
      Double_t quantil[2] = {(1-level)/2,(1+level)/2};
      pdf->GetQuantiles(2,q,quantil);
      eff[i-1] = pdf->Mean(0,1);
      efflow[i-1]= eff[i-1] - q[0];
      effhigh[i-1]= q[1] - eff[i-1];

      delete pdf;
      pdf = 0;      
   }//loop over all bins

   TGraphAsymmErrors* gr = new TGraphAsymmErrors(nbins_max,x,eff,xlow,xhigh,efflow,effhigh);

   delete [] x;
   delete [] xlow;
   delete [] xhigh;
   delete [] eff;
   delete [] efflow;
   delete [] effhigh;

   return gr;
}

//______________________________________________________________________________
void TEfficiency::Draw(const Option_t* opt)
{
   //draws the current TEfficiency object
   //
   //options:
   //- 1-dimensional case: same options as TGraphAsymmErrors::Draw()
   //- 2-dimensional case: same options as TH2::Draw()
   //- 3-dimensional case: not yet supported
   
   //check options
   TString option = opt;
   option.ToLower();

   if(gPad && !option.Contains("same"))
      gPad->Clear();

   AppendPad(option.Data());
}

//______________________________________________________________________________
void TEfficiency::Fill(Bool_t bPassed,Double_t x,Double_t y,Double_t z)
{
   //This function is used for filling the two histograms.
   //
   //Input: bPassed - flag whether the current event passed the selection
   //                 true: both histograms are filled
   //                 false: only the total histogram is filled
   //       x       - x value
   //       y       - y value (use default=0 for 1-D efficiencies)
   //       z       - z value (use default=0 for 2-D or 1-D efficiencies)

   switch(GetDimension()) {
   case 1:
      fTotalHistogram->Fill(x);
      if(bPassed)
	 fPassedHistogram->Fill(x);
      break;
   case 2:
      ((TH2*)(fTotalHistogram))->Fill(x,y);
      if(bPassed)
	 ((TH2*)(fPassedHistogram))->Fill(x,y);
      break;
   case 3:
      ((TH3*)(fTotalHistogram))->Fill(x,y,z);
      if(bPassed)
	 ((TH3*)(fPassedHistogram))->Fill(x,y,z);
      break;
   }
}

//______________________________________________________________________________
Int_t TEfficiency::FindFixBin(Double_t x,Double_t y,Double_t z) const
{
   //returns the global bin number containing the given values
   //
   //Note: - values which belong to dimensions higher than the current dimension
   //        of the TEfficiency object are ignored (i.e. for 1-dimensional
   //        efficiencies only the x-value is considered)
   
   Int_t nx = fTotalHistogram->GetXaxis()->FindFixBin(x);
   Int_t ny = 0;
   Int_t nz = 0;
   
   switch(GetDimension()) {
   case 3: nz = fTotalHistogram->GetZaxis()->FindFixBin(z);
   case 2: ny = fTotalHistogram->GetYaxis()->FindFixBin(y);break;
   }

   return GetGlobalBin(nx,ny,nz);
}

//______________________________________________________________________________
Int_t TEfficiency::Fit(TF1* f1,Option_t* opt)
{
   //fits the efficiency using the TBinomialEfficiencyFitter class
   //
   //The resulting fit function is added to the list of associated functions.
   //
   //Options: - "+": previous fitted functions in the list are kept, by default
   //                all functions in the list are deleted
   //         - for more fitting options see TBinomialEfficiencyFitter::Fit

   TString option = opt;
   option.ToLower();

   //replace existing functions in list with same name
   Bool_t bDeleteOld = true;
   if(option.Contains("+")) {
      option.ReplaceAll("+","");
      bDeleteOld = false;
   }
   
   TBinomialEfficiencyFitter Fitter(fPassedHistogram,fTotalHistogram);
   
   Int_t result = Fitter.Fit(f1,option.Data());

   //create copy which is appended to the list
   TF1* pFunc = new TF1(*f1);
   
   if(!fFunctions)
      fFunctions = new TList();

   if(bDeleteOld) {
      TIter next(fFunctions);
      TObject* obj = 0;
      while((obj = next())) {
	 if(obj->InheritsFrom(TF1::Class())) {
	    fFunctions->Remove(obj);
	    delete obj;
	 }
      }      
   }
   
   fFunctions->Add(pFunc);
   
   return result;
}

//______________________________________________________________________________
TH1* TEfficiency::GetCopyPassedHisto() const
{
   //returns a cloned version of the histogram containing the passed events
   //
   //Notes: - The histogram is filled with unit weights. You might want to scale
   //         it with the global weight GetWeight().
   //       - The returned object is owned by the user who has to care about the
   //         deletion of the new TH1 object.
   //       - This histogram is by default NOT attached to the current directory
   //         to avoid duplication of data. If you want to store it automatically
   //         during the next TFile::Write() command, you have to attach it to
   //         the corresponding directory.
   //Begin_html
   //<div class="code"><pre>
   //  TFile* pFile = new TFile("passed.root","update");
   //  TEfficiency* pEff = (TEfficiency*)gDirectory->Get("my_eff");
   //  TH1* copy = pEff->GetCopyPassedHisto();
   //  copy->SetDirectory(gDirectory);
   //  pFile->Write();
   //</pre></div>
   //<div class="clear"></div>
   //End_Html
    
   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   TH1* tmp = (TH1*)(fPassedHistogram->Clone());
   TH1::AddDirectory(bStatus);

   return tmp;
}

//______________________________________________________________________________
TH1* TEfficiency::GetCopyTotalHisto() const
{
   //returns a cloned version of the histogram containing all events
   //
   //Notes: - The histogram is filled with unit weights. You might want to scale
   //         it with the global weight GetWeight().
   //       - The returned object is owned by the user who has to care about the
   //         deletion of the new TH1 object.
   //       - This histogram is by default NOT attached to the current directory
   //         to avoid duplication of data. If you want to store it automatically
   //         during the next TFile::Write() command, you have to attach it to
   //         the corresponding directory.
   //Begin_Html
   //<div class="code"><pre>
   //  TFile* pFile = new TFile("total.root","update");
   //  TEfficiency* pEff = (TEfficiency*)gDirectory->Get("my_eff");
   //  TH1* copy = pEff->GetCopyTotalHisto();
   //  copy->SetDirectory(gDirectory);
   //  pFile->Write();
   //</pre></div>
   //<div class="clear"></div>
   //End_Html
   
   Bool_t bStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE);
   TH1* tmp = (TH1*)(fTotalHistogram->Clone());
   TH1::AddDirectory(bStatus);

   return tmp;
}

//______________________________________________________________________________
Int_t TEfficiency::GetDimension() const
{
   //returns the dimension of the current TEfficiency object   
   
   return fTotalHistogram->GetDimension();
}

//______________________________________________________________________________
Double_t TEfficiency::GetEfficiency(Int_t bin) const
{
   //returns the efficiency in the given global bin
   //
   //Note: - The estimated efficiency depends on the chosen statistic option:
   //        for frequentist ones:
   //        Begin_Latex #hat{#varepsilon} = #frac{passed}{total} End_Latex
   //        for bayesian ones the expectation value of the resulting posterior
   //        distribution is returned:
   //        Begin_Latex #hat{#varepsilon} = #frac{passed + #alpha}{total + #alpha + #beta} End_Latex
   //      - If the denominator is equal to 0, an efficiency of 0 is returned.
   
   Int_t total = fTotalHistogram->GetBinContent(bin);
   Int_t passed = fPassedHistogram->GetBinContent(bin);
      
   if(TestBit(kIsBayesian))
      return (total + fBeta_alpha + fBeta_beta)?
	 (passed + fBeta_alpha)/(total + fBeta_alpha + fBeta_beta): 0;
   else
      return (total)? ((Double_t)passed)/total : 0;
}

//______________________________________________________________________________
Double_t TEfficiency::GetEfficiencyErrorLow(Int_t bin) const
{
   //returns the lower error on the efficiency in the given global bin
   //
   //The result depends on the current confidence level fConfLevel and the
   //chosen statistic option fStatisticOption. See SetStatisticOption(Int_t) for
   //more details.

   Int_t total = fTotalHistogram->GetBinContent(bin);
   Int_t passed = fPassedHistogram->GetBinContent(bin);

   Double_t eff = GetEfficiency(bin);

   if(TestBit(kIsBayesian))
      return (eff - BayesianLow(total,passed,fConfLevel,fBeta_alpha,fBeta_beta));
   else
      return (eff - fLowerBound(total,passed,fConfLevel));
}

//______________________________________________________________________________
Double_t TEfficiency::GetEfficiencyErrorUp(Int_t bin) const
{   
   //returns the upper error on the efficiency in the given global bin
   //
   //The result depends on the current confidence level fConfLevel and the
   //chosen statistic option fStatisticOption. See SetStatisticOption(Int_t) for
   //more details.
   
   Int_t total = fTotalHistogram->GetBinContent(bin);
   Int_t passed = fPassedHistogram->GetBinContent(bin);

   Double_t eff = GetEfficiency(bin);

   if(TestBit(kIsBayesian))
      return (BayesianUp(total,passed,fConfLevel,fBeta_alpha,fBeta_beta) - eff);
   else
      return fUpperBound(total,passed,fConfLevel) - eff;
}

//______________________________________________________________________________
Int_t TEfficiency::GetGlobalBin(Int_t binx,Int_t biny,Int_t binz) const
{
   //returns the global bin number which can be used as argument for the
   //following functions:
   //
   // - GetEfficiency(bin), GetEfficiencyErrorLow(bin), GetEfficiencyErrorUp(bin)
   // - GetPassedEvents(bin), SetPassedEvents(bin), GetTotalEvents(bin),
   //   SetTotalEvents(bin)
   //
   //see TH1::GetBin() for conventions on numbering bins
   
   return fTotalHistogram->GetBin(binx,biny,binz);
}

//______________________________________________________________________________
Int_t TEfficiency::GetPassedEvents(Int_t bin) const
{
   //returns the number of selected events in the given global bin
   
   return fPassedHistogram->GetBinContent(bin);
}

//______________________________________________________________________________
Int_t TEfficiency::GetTotalEvents(Int_t bin) const
{
   //returns the number of total events in the given global bin
   
   return fTotalHistogram->GetBinContent(bin);
}

//______________________________________________________________________________
void TEfficiency::Merge(TCollection* pList)
{
   //merges the TEfficiency objects in the given list to the given
   //TEfficiency object using the operator+=(TEfficiency&)
   //
   //The merged result is stored in the current object. The statistic options and
   //the confidence level are taken from the current object.
   //
   //This function should be used when all TEfficiency objects correspond to
   //the same process.
   //
   //The new weight is set according to:
   //Begin_Latex #frac{1}{w_{new}} = #sum_{i} \frac{1}{w_{i}}End_Latex 
   
   if(pList->IsEmpty())
      return;
   
   TIter next(pList);
   TObject* obj = 0;
   TEfficiency* pEff = 0;
   while((obj = next())) {
      pEff = dynamic_cast<TEfficiency*>(obj);
      if(pEff)
	 *this += *pEff;
   }
}

//______________________________________________________________________________
Double_t TEfficiency::NormalLow(Int_t total,Int_t passed,Double_t level)
{
   //returns the lower confidence limit for the efficiency supposing that the
   //efficiency follows a normal distribution with the rms below
   //Begin_Latex(separator='=',align='rl')
   // #hat{#varepsilon} = #frac{passed}{total}
   // #sigma_{#varepsilon} = #sqrt{#frac{#hat{#varepsilon} ( 1-#hat{#varepsilon})}{total}}
   // #varepsilon_{low} = #hat{#varepsilon} + #Phi^{-1}(#frac{level}{2},#sigma_{#varepsilon})
   //End_Latex

   Double_t alpha = (1.0 - level)/2;
   Double_t average = ((Double_t)passed) / total;
   Double_t sigma = std::sqrt(average * (1 - average) / total);
   Double_t delta = ROOT::Math::normal_quantile(1 - alpha,sigma);

   return ((average - delta) < 0) ? 0.0 : (average - delta);
}

//______________________________________________________________________________
Double_t TEfficiency::NormalUp(Int_t total,Int_t passed,Double_t level)
{
   //returns the upper confidence limit for the efficiency supposing that the
   //efficiency follows a normal distribution with the rms below
   //Begin_Latex(separator='=',align='rl')
   // #hat{#varepsilon} = #frac{passed}{total}
   // #sigma_{#varepsilon} = #sqrt{#frac{#hat{#varepsilon} ( 1-#hat{#varepsilon})}{total}}
   // #varepsilon_{up} = #hat{#varepsilon} + #Phi^{-1}(1 - #frac{level}{2},#sigma_{#varepsilon})
   //End_Latex
   
   Double_t alpha = (1.0 - level)/2;
   Double_t average = ((Double_t)passed) / total;
   Double_t sigma = std::sqrt(average * (1 - average) / total);
   Double_t delta = ROOT::Math::normal_quantile(1 - alpha,sigma);

   return ((average + delta) > 1) ? 1.0 : (average + delta);   
}

//______________________________________________________________________________
TEfficiency& TEfficiency::operator+=(const TEfficiency& rhs)
{
   //adds the histograms of another TEfficiency object to current histograms
   //
   //The statistic options and the confidence level remain unchanged.
   //
   //fTotalHistogram += rhs.fTotalHistogram;
   //fPassedHistogram += rhs.fPassedHistogram;
   //
   //calculates a new weight:
   //current weight of this TEfficiency object = Begin_Latex w_{1} End_Latex
   //weight of rhs = Begin_Latex w_{2} End_Latex
   //Begin_Latex w_{new} = \frac{w_{1} \times w_{2}}{w_{1} + w_{2}}End_Latex
   
   fTotalHistogram->ResetBit(TH1::kIsAverage);
   fPassedHistogram->ResetBit(TH1::kIsAverage);

   fTotalHistogram->Add(rhs.fTotalHistogram);
   fPassedHistogram->Add(rhs.fPassedHistogram);

   SetWeight((fWeight * rhs.GetWeight())/(fWeight + rhs.GetWeight()));
   
   return *this;
}

//______________________________________________________________________________
TEfficiency& TEfficiency::operator=(const TEfficiency& rhs)
{
   //assignment operator
   //
   //The histograms, statistic option, confidence level, weight and paint styles
   //of rhs are copied to the this TEfficiency object.
   //
   //Note: - The list of associated functions is not copied. After this
   //        operation the list of associated functions is empty.
   
   if(this != &rhs)
   {
      //statistic options
      SetStatisticOption(rhs.GetStatisticOption());
      SetConfidenceLevel(rhs.GetConfidenceLevel());
      SetBetaAlpha(rhs.GetBetaAlpha());
      SetBetaBeta(rhs.GetBetaBeta());
      SetWeight(rhs.GetWeight());
      
      //associated list of functions
      if(fFunctions)
	 fFunctions->Delete();

      //copy histograms
      delete fTotalHistogram;
      delete fPassedHistogram;

      Bool_t bStatus = TH1::AddDirectoryStatus();
      TH1::AddDirectory(kFALSE);
      fTotalHistogram = (TH1*)(rhs.fTotalHistogram->Clone());
      fPassedHistogram = (TH1*)(rhs.fPassedHistogram->Clone());
      TH1::AddDirectory(bStatus);

      //delete temporary paint objects
      delete fPaintHisto;
      delete fPaintGraph;
      fPaintHisto = 0;
      fPaintGraph = 0;

      //copy style
      SetLineColor(rhs.GetLineColor());
      SetLineStyle(rhs.GetLineStyle());
      SetLineWidth(rhs.GetLineWidth());
      SetFillColor(rhs.GetFillColor());
      SetFillStyle(rhs.GetFillStyle());
      SetMarkerColor(rhs.GetMarkerColor());
      SetMarkerSize(rhs.GetMarkerSize());
      SetMarkerStyle(rhs.GetMarkerStyle());
   }

   return *this;
}

//______________________________________________________________________________
void TEfficiency::Paint(const Option_t* opt)
{
   //paints this TEfficiency object
   //
   //For details on the possible option see Draw(Option_t*)
   
   if(!gPad)
      return;
   
   TString option = opt;
   option.ToLower();

   //use TGraphAsymmErrors for painting
   if(GetDimension() == 1) {
      Int_t npoints = fTotalHistogram->GetNbinsX();
      if(!fPaintGraph) {
	 fPaintGraph = new TGraphAsymmErrors(npoints);
	 fPaintGraph->SetName("eff_graph");
      }
      //refresh title before painting
      fPaintGraph->SetTitle(GetTitle());

      //errors for points
      Double_t xlow,xup,ylow,yup;
      //point i corresponds to bin i+1 in histogram
      for(Int_t i = 0; i < npoints; ++i) {
	 fPaintGraph->SetPoint(i,fTotalHistogram->GetBinCenter(i+1),GetEfficiency(i+1));
	 xlow = fTotalHistogram->GetBinCenter(i+1) - fTotalHistogram->GetBinLowEdge(i+1);
	 xup = fTotalHistogram->GetBinWidth(i+1) - xlow;
	 ylow = GetEfficiencyErrorLow(i+1);
	 yup = GetEfficiencyErrorUp(i+1);
	 fPaintGraph->SetPointError(i,xlow,xup,ylow,yup);
      }

      //copying style information
      fPaintGraph->SetLineColor(GetLineColor());
      fPaintGraph->SetLineStyle(GetLineStyle());
      fPaintGraph->SetLineWidth(GetLineWidth());
      fPaintGraph->SetFillColor(GetFillColor());
      fPaintGraph->SetFillStyle(GetFillStyle());
      fPaintGraph->SetMarkerColor(GetMarkerColor());
      fPaintGraph->SetMarkerSize(GetMarkerSize());
      fPaintGraph->SetMarkerStyle(GetMarkerStyle());
      
      //paint graph      
      fPaintGraph->Paint(option.Data());

      //paint all associated functions
      if(fFunctions) {
	 //paint box with fit parameters
	 gStyle->SetOptFit(1);
	 TIter next(fFunctions);
	 TObject* obj = 0;
	 while((obj = next())) {
	    if(obj->InheritsFrom(TF1::Class())) {
               fPaintGraph->PaintStats((TF1*)obj);
	       ((TF1*)obj)->Paint("sameC");
	    }
	 }
      }
      
      return;
   }

   //use TH2 for painting
   if(GetDimension() == 2) {
      Int_t nbinsx = fTotalHistogram->GetNbinsX();
      Int_t nbinsy = fTotalHistogram->GetNbinsY();
      if(!fPaintHisto) {
	 fPaintHisto = new TH2F("eff_histo",GetTitle(),nbinsx,fTotalHistogram->GetXaxis()->GetXbins()->GetArray(),
				nbinsy,fTotalHistogram->GetYaxis()->GetXbins()->GetArray());
	 fPaintHisto->SetDirectory(0);
      }
      //refresh title before each painting
      fPaintHisto->SetTitle(GetTitle());

      Int_t bin;
      for(Int_t i = 0; i < nbinsx + 2; ++i) {
	 for(Int_t j = 0; j < nbinsy + 2; ++j) {
	    bin = GetGlobalBin(i,j);
	    fPaintHisto->SetBinContent(bin,GetEfficiency(bin));
	 }
      }

      //copying style information
      fPaintHisto->SetLineColor(GetLineColor());
      fPaintHisto->SetLineStyle(GetLineStyle());
      fPaintHisto->SetLineWidth(GetLineWidth());
      fPaintHisto->SetFillColor(GetFillColor());
      fPaintHisto->SetFillStyle(GetFillStyle());
      fPaintHisto->SetMarkerColor(GetMarkerColor());
      fPaintHisto->SetMarkerSize(GetMarkerSize());
      fPaintHisto->SetMarkerStyle(GetMarkerStyle());
      fPaintHisto->SetStats(0);

      //paint histogram
      fPaintHisto->Paint(option.Data());
      return;
   }
}

//______________________________________________________________________________
void TEfficiency::SetBetaAlpha(Double_t alpha)
{
   //sets the shape parameter Begin_Latex \alpha End_Latex
   //
   //The prior probability of the efficiency is given by the beta distribution:
   //Begin_Latex
   // f(\varepsilon;\alpha;\beta) = \frac{1}{B(\alpha,\beta)} \varepsilon^{\alpha-1} (1 - \varepsilon)^{\beta-1}
   //End_Latex
   //
   //Note: - both shape parameters have to be positive (i.e. > 0)
   
   if(alpha > 0)
      fBeta_alpha = alpha;
   else
      Warning("SetBetaAlpha(Double_t)","invalid shape parameter %.2lf",alpha);
}

//______________________________________________________________________________
void TEfficiency::SetBetaBeta(Double_t beta)
{   
   //sets the shape parameter Begin_Latex \beta End_Latex
   //
   //The prior probability of the efficiency is given by the beta distribution:
   //Begin_Latex
   // f(\varepsilon;\alpha,\beta) = \frac{1}{B(\alpha,\beta)} \varepsilon^{\alpha-1} (1 - \varepsilon)^{\beta-1}
   //End_Latex
   //
   //Note: - both shape parameters have to be positive (i.e. > 0)
 
   if(beta > 0)
      fBeta_beta = beta;
   else
      Warning("SetBetaBeta(Double_t)","invalid shape parameter %.2lf",beta);
}

//______________________________________________________________________________
void TEfficiency::SetConfidenceLevel(Double_t level)
{
   //sets the confidence level (0 < level < 1)

   if((level > 0) && (level < 1))
      fConfLevel = level;
   else
      Warning("SetConfidenceLevel(Double_t)","invalid confidence level %.2lf",level);
}

//______________________________________________________________________________
void TEfficiency::SetDirectory(TDirectory* dir)
{
   //sets the directory holding this TEfficiency object
   //
   //A reference to this TEfficiency object is removed from the current
   //directory (if it exists) and a new reference to this TEfficiency object is
   //added to the given directory.
   //
   //Notes: - If the given directory is 0, the TEfficiency object does not
   //         belong to any directory and will not be written to file during the
   //         next TFile::Write() command.

   if(fDirectory == dir)
      return;
   if(fDirectory)
      fDirectory->Remove(this);
   fDirectory = dir;
   if(fDirectory)
      fDirectory->Append(this);      
}

//______________________________________________________________________________
void TEfficiency::SetName(const char* name)
{
   //sets the name
   //
   //Note: The names of the internal histograms are set to "name + _total" or
   //      "name + _passed" respectively.
   
   TNamed::SetName(name);
   
   //setting the names (appending the correct ending)
   char* namebuf = new char[strlen(name)+8];
   strcpy(namebuf,name);
   strcat(namebuf,"_total");
   fTotalHistogram->SetName(namebuf);
   strcpy(namebuf,name);
   strcat(namebuf,"_passed");
   fPassedHistogram->SetName(namebuf);
   delete namebuf;
}

//______________________________________________________________________________
void TEfficiency::SetPassedEvents(Int_t bin,Int_t events)
{
   //sets the number of passed events in the given global bin
   //
   //Note: - requires: 0 <= events <= GetTotalEvents(bin)

   if(events <= GetTotalEvents(bin))
      fPassedHistogram->SetBinContent(bin,events);
   else
      Error("SetPassedEvents(Int_t,Int_t)","total number of events (%i) in bin %i is less than given number of passed events %i",GetTotalEvents(bin),bin,events);
}

//______________________________________________________________________________
Bool_t TEfficiency::SetPassedHistogram(const TH1& rPassed,Option_t* opt)
{
   //sets the histogram containing the passed events
   //
   //The given histogram is cloned and stored internally as histogram containing
   //the passed events. The given histogram has to be consistent with the current
   //fTotalHistogram (see CheckConsistency(const TH1&,const TH1&)).
   //The method returns whether the fPassedHistogram has been replaced (true) or
   //not (false).
   //
   //Note: The list of associated functions fFunctions is cleared.
   //
   //Option: - "f": force the replacement without checking the consistency
   //               This can lead to inconsistent histograms and useless results
   //               or unexpected behaviour. But sometimes it might be the only
   //               way to change the histograms. If you use this option, you
   //               should ensure that the fTotalHistogram is replaced by a
   //               consistent one (with respect to rPassed) as well.

   TString option = opt;
   option.ToLower();

   Bool_t bReplace = option.Contains("f");

   if(!bReplace)
      bReplace = CheckConsistency(rPassed,*fTotalHistogram);

   if(bReplace) {
      delete fPassedHistogram;
      Bool_t bStatus = TH1::AddDirectoryStatus();
      TH1::AddDirectory(kFALSE);
      fPassedHistogram = (TH1*)(rPassed.Clone());
      TH1::AddDirectory(bStatus);

      if(fFunctions)
	 fFunctions->Delete();

      return true;
   }
   else
      return false;
}

//______________________________________________________________________________
void TEfficiency::SetStatisticOption(Int_t option)
{
   //description still missing
   
   fStatisticOption = option;

   switch(option)
   {
   case kFCP:      
      fUpperBound = &ClopperPearsonUp;
      fLowerBound = &ClopperPearsonLow;
      SetBit(kIsBayesian,false);
      break;
   case kFNormal:      
      fUpperBound = &NormalUp;
      fLowerBound = &NormalLow;
      SetBit(kIsBayesian,false);
      break;
   case kFWilson:      
      fUpperBound = &WilsonUp;
      fLowerBound = &WilsonLow;
      SetBit(kIsBayesian,false);
      break;
   case kFAC:
      fUpperBound = &AgrestiCoullUp;
      fLowerBound = &AgrestiCoullLow;
      SetBit(kIsBayesian,false);
      break;
   case kBJeffrey:
      fBeta_alpha = 0.5;
      fBeta_beta = 0.5;
      SetBit(kIsBayesian,true);
      break;
   case kBUniform:
      fBeta_alpha = 1;
      fBeta_beta = 1;
      SetBit(kIsBayesian,true);
      break;
   case kBBayesian:
      SetBit(kIsBayesian,true);
      break;
   default:
      fStatisticOption = kFCP;
      fUpperBound = &ClopperPearsonUp;
      fLowerBound = &ClopperPearsonLow;
      SetBit(kIsBayesian,false);
   }
}

//______________________________________________________________________________
void TEfficiency::SetTitle(const char* title)
{
   //sets the title
   //
   //Notes: - The titles of the internal histograms are set to "title + (total)"
   //         or "title + (passed)" respectively.
   //       - It is possible to label the axis of the histograms as usual (see
   //         TH1::SetTitle).
   //
   //Example: Setting the title to "My Efficiency" and label the axis
   //Begin_Html
   //<div class="code"><pre>
   //pEff->SetTitle("My Efficiency;x label;eff");
   //</pre></div>
   //<div class="clear"></div>
   //End_Html
   
   TNamed::SetTitle(title);
   
   //setting the titles (looking for the first semicolon and insert the tokens there)
   char* titlebuf = new char[strlen(title)+10];
   strcpy(titlebuf,title);
   char* semicolon = strchr(titlebuf,';');
   if(semicolon != 0)
   {
      char* axistitles = new char[strlen(titlebuf)];
      strcpy(axistitles,semicolon);
      
      strcpy(semicolon," (total)\0");
      strcat(titlebuf,axistitles);
      fTotalHistogram->SetTitle(titlebuf);
      strcpy(semicolon," (passed)\0");
      strcat(titlebuf,axistitles);
      fPassedHistogram->SetTitle(titlebuf);

      delete [] axistitles;
   }
   else
   {
      strcat(titlebuf," (total)");
      fTotalHistogram->SetTitle(titlebuf);
      strcpy(titlebuf,title);
      strcat(titlebuf," (passed)");
      fPassedHistogram->SetTitle(titlebuf);
   }

   delete [] titlebuf;
}

//______________________________________________________________________________
void TEfficiency::SetTotalEvents(Int_t bin,Int_t events)
{
   //sets the number of total events in the given global bin
   //
   //Note: - requires: GetPassedEvents(bin) <= events

   if(events >= GetPassedEvents(bin))
      fTotalHistogram->SetBinContent(bin,events);
   else
      Error("SetTotalEvents(Int_t,Int_t)","passed number of events (%i) in bin %i is bigger than given number of total events %i",GetPassedEvents(bin),bin,events);
}

//______________________________________________________________________________
Bool_t TEfficiency::SetTotalHistogram(const TH1& rTotal,Option_t* opt)
{
   //sets the histogram containing all events
   //
   //The given histogram is cloned and stored internally as histogram containing
   //all events. The given histogram has to be consistent with the current
   //fPassedHistogram (see CheckConsistency(const TH1&,const TH1&)).
   //The method returns whether the fTotalHistogram has been replaced (true) or
   //not (false).
   //
   //Note: The list of associated functions fFunctions is cleared.
   //
   //Option: - "f": force the replacement without checking the consistency
   //               This can lead to inconsistent histograms and useless results
   //               or unexpected behaviour. But sometimes it might be the only
   //               way to change the histograms. If you use this option, you
   //               should ensure that the fPassedHistogram is replaced by a
   //               consistent one (with respect to rTotal) as well.

   TString option = opt;
   option.ToLower();

   Bool_t bReplace = option.Contains("f");

   if(!bReplace)
      bReplace = CheckConsistency(*fPassedHistogram,rTotal);

   if(bReplace) {
      delete fTotalHistogram;
      Bool_t bStatus = TH1::AddDirectoryStatus();
      TH1::AddDirectory(kFALSE);
      fTotalHistogram = (TH1*)(rTotal.Clone());
      TH1::AddDirectory(bStatus);

      if(fFunctions)
	 fFunctions->Delete();

      return true;
   }
   else
      return false;
}

//______________________________________________________________________________
void TEfficiency::SetWeight(Double_t weight)
{
   //sets the global weight for this TEfficiency object
   //
   //Note: - weight has to be positive ( > 0)
   
   if(weight > 0)
      fWeight = weight;
   else
      Warning("SetWeight","invalid weight %.2lf",weight);
}

//______________________________________________________________________________
Double_t TEfficiency::WilsonLow(Int_t total,Int_t passed,Double_t level)
{
   //calculates the lower boundary for the frequentist Wilson interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //Begin_Latex(separator='=',align='rl')
   // #alpha = 1 - #frac{level}{2}
   // #kappa = #Phi^{-1}(1 - #alpha,1) ... normal quantile function
   // mode = #frac{passed + #frac{#kappa^{2}}{2}}{total + #kappa^{2}}
   // #Delta = #frac{#kappa}{total + #kappa^{2}} * #sqrt{passed (1 - #frac{passed}{total}) + #frac{#kappa^{2}}{4}}
   // return =  max(0,mode - #Delta)
   //End_Latex
   
   Double_t alpha = (1.0 - level)/2;
   Double_t average = ((Double_t)passed) / total;
   Double_t kappa = ROOT::Math::normal_quantile(1 - alpha,1);

   Double_t mode = (passed + 0.5 * kappa * kappa) / (total + kappa * kappa);
   Double_t delta = kappa / (total + kappa*kappa) * std::sqrt(total * average
							      * (1 - average) + kappa * kappa / 4);
   return ((mode - delta) < 0) ? 0.0 : (mode - delta);
}

//______________________________________________________________________________
Double_t TEfficiency::WilsonUp(Int_t total,Int_t passed,Double_t level)
{
   //calculates the upper boundary for the frequentist Wilson interval
   //
   //Input: - total : number of total events
   //       - passed: 0 <= number of passed events <= total
   //       - level : confidence level
   //
   //calculation:
   //Begin_Latex(separator='=',align='rl')
   // #alpha = 1 - #frac{level}{2}
   // #kappa = #Phi^{-1}(1 - #alpha,1) ... normal quantile function
   // mode = #frac{passed + #frac{#kappa^{2}}{2}}{total + #kappa^{2}}
   // #Delta = #frac{#kappa}{total + #kappa^{2}} * #sqrt{passed (1 - #frac{passed}{total}) + #frac{#kappa^{2}}{4}}
   // return =  min(1,mode + #Delta)
   //End_Latex
   
   Double_t alpha = (1.0 - level)/2;
   Double_t average = ((Double_t)passed) / total;
   Double_t kappa = ROOT::Math::normal_quantile(1 - alpha,1);

   Double_t mode = (passed + 0.5 * kappa * kappa) / (total + kappa * kappa);
   Double_t delta = kappa / (total + kappa*kappa) * std::sqrt(total * average
							      * (1 - average) + kappa * kappa / 4);
   return ((mode + delta) > 1) ? 1.0 : (mode + delta);
}

//______________________________________________________________________________
const TEfficiency operator+(const TEfficiency& lhs,const TEfficiency& rhs)
{
   // addition operator
   //
   // adds the corresponding histograms:
   // lhs.GetTotalHistogram() + rhs.GetTotalHistogram()
   // lhs.GetPassedHistogram() + rhs.GetPassedHistogram()
   //
   // the statistic option and the confidence level are taken from lhs
   
   TEfficiency tmp(lhs);
   tmp += rhs;
   return tmp;
}

#endif
