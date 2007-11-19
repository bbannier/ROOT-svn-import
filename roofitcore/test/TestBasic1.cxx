#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooDataSet.h"
#include "RooPlot.h"

// Elementary operations on a gaussian PDF
class TestBasic1 : public RooFitTestUnit
{
public: 
  TestBasic1(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooFitTestUnit(refFile,writeRef,verbose) {} ;
  Bool_t testCode() {

    // Build Gaussian PDF
    RooRealVar x("x","x",-10,10) ;
    RooRealVar mean("mean","mean of gaussian",-1) ;
    RooRealVar sigma("sigma","width of gaussian",3) ;
    RooGaussian gauss("gauss","gaussian PDF",x,mean,sigma) ;  
    
    // Plot PDF
    RooPlot* xframe = x.frame() ;
    gauss.plotOn(xframe) ;
    
    // Generate a toy MC set
    RooDataSet* data = gauss.generate(x,10000) ;  
    
    // Fit pdf to toy
    mean.setConstant(kFALSE) ;
    sigma.setConstant(kFALSE) ;
    RooFitResult* r = gauss.fitTo(*data,"mhr") ;

    // Plot PDF and toy data overlaid
    RooPlot* xframe2 = x.frame() ;
    data->plotOn(xframe2) ;
    gauss.plotOn(xframe2) ;
     
    // Register output frame for comparison test
    regResult(r,"Basic1_Result") ;
    regPlot(xframe2,"Basic1_Plot") ;
 
    return kTRUE ;
  }
} ;
