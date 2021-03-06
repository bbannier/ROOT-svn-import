// new HFit function 
//______________________________________________________________________________


#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TF2.h"
#include "TF3.h"
#include "TError.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraph2D.h"
#include "THnBase.h"

#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "HFitInterface.h"
#include "Math/MinimizerOptions.h"
#include "Math/Minimizer.h"

#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"

#include "TList.h"
#include "TMath.h"

#include "TClass.h"
#include "TVirtualPad.h" // for gPad

#include "TBackCompFitter.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <stdlib.h>
#include <cmath>
#include <memory>
#include <limits>

//#define DEBUG

// utility functions used in TH1::Fit

namespace HFit { 


   int GetDimension(const TH1 * h1) { return h1->GetDimension(); }
   int GetDimension(const TGraph * ) { return 1; }
   int GetDimension(const TMultiGraph * ) { return 1; }
   int GetDimension(const TGraph2D * ) { return 2; }
   int GetDimension(const THnBase * s1) { return s1->GetNdimensions(); }

   int CheckFitFunction(const TF1 * f1, int hdim);

   void CheckGraphFitOptions(Foption_t &fitOption);


   void GetDrawingRange(TH1 * h1, ROOT::Fit::DataRange & range);
   void GetDrawingRange(TGraph * gr, ROOT::Fit::DataRange & range);
   void GetDrawingRange(TMultiGraph * mg, ROOT::Fit::DataRange & range);
   void GetDrawingRange(TGraph2D * gr, ROOT::Fit::DataRange & range);
   void GetDrawingRange(THnBase * s, ROOT::Fit::DataRange & range);


   template <class FitObject>
   TFitResultPtr Fit(FitObject * h1, TF1 *f1 , Foption_t & option , const ROOT::Math::MinimizerOptions & moption, const char *goption,  ROOT::Fit::DataRange & range); 

   template <class FitObject>
   void StoreAndDrawFitFunction(FitObject * h1, TF1 * f1, const ROOT::Fit::DataRange & range, bool, bool, const char *goption);


} 

int HFit::CheckFitFunction(const TF1 * f1, int dim) { 
   // Check validity of fitted function
   if (!f1) {
      Error("Fit", "function may not be null pointer");
      return -1;
   }
   if (f1->IsZombie()) {
      Error("Fit", "function is zombie");
      return -2;
   }

   int npar = f1->GetNpar();
   if (npar <= 0) {
      Error("Fit", "function %s has illegal number of parameters = %d", f1->GetName(), npar);
      return -3;
   }

   // Check that function has same dimension as histogram
   if (f1->GetNdim() > dim) {
      Error("Fit","function %s dimension, %d, is greater than fit object dimension, %d",
            f1->GetName(), f1->GetNdim(), dim);
      return -4;
   }
   if (f1->GetNdim() < dim-1) {
      Error("Fit","function %s dimension, %d, is smaller than fit object dimension -1, %d",
            f1->GetName(), f1->GetNdim(), dim);
      return -5;
   }

   return 0; 

}

template<class FitObject>
TFitResultPtr HFit::Fit(FitObject * h1, TF1 *f1 , Foption_t & fitOption , const ROOT::Math::MinimizerOptions & minOption, const char *goption, ROOT::Fit::DataRange & range)
{
   // perform fit of histograms, or graphs using new fitting classes 
   // use same routines for fitting both graphs and histograms

#ifdef DEBUG
   printf("fit function %s\n",f1->GetName() ); 
#endif

   // replacement function using  new fitter
   int hdim = HFit::GetDimension(h1); 
   int iret = HFit::CheckFitFunction(f1, hdim);
   if (iret != 0) return iret; 



   // integral option is not supported in this case
   if (f1->GetNdim() < hdim ) { 
      if (fitOption.Integral) Info("Fit","Ignore Integral option. Model function dimension is less than the data object dimension");
      if (fitOption.Like) Info("Fit","Ignore Likelihood option. Model function dimension is less than the data object dimension");
      fitOption.Integral = 0; 
      fitOption.Like = 0; 
   }

   Int_t special = f1->GetNumber();
   Bool_t linear = f1->IsLinear();
   Int_t npar = f1->GetNpar();
   if (special==299+npar)  linear = kTRUE; // for polynomial functions 
   // do not use linear fitter in these case
   if (fitOption.Bound || fitOption.Like || fitOption.Errors || fitOption.Gradient || fitOption.More || fitOption.User|| fitOption.Integral || fitOption.Minuit)
      linear = kFALSE;


   // create the fitter
   std::auto_ptr<ROOT::Fit::Fitter> fitter(new ROOT::Fit::Fitter() );
   ROOT::Fit::FitConfig & fitConfig = fitter->Config();

   // create options 
   ROOT::Fit::DataOptions opt; 
   opt.fIntegral = fitOption.Integral; 
   opt.fUseRange = fitOption.Range; 
   opt.fExpErrors = fitOption.PChi2;  // pearson chi2 with expected errors
   if (fitOption.Like || fitOption.PChi2) opt.fUseEmpty = true;  // use empty bins in log-likelihood fits 
   if (special==300) opt.fCoordErrors = false; // no need to use coordinate errors in a pol0 fit
   if (fitOption.NoErrX) opt.fCoordErrors = false;  // do not use coordinate errors when requested
   if (fitOption.W1 ) opt.fErrors1 = true;
   if (fitOption.W1 > 1) opt.fUseEmpty = true; // use empty bins with weight=1

   //opt.fBinVolume = 1; // for testing

   if (opt.fUseRange) { 
#ifdef DEBUG
      printf("use range \n" ); 
#endif
      // check if function has range 
      Double_t fxmin, fymin, fzmin, fxmax, fymax, fzmax;
      f1->GetRange(fxmin, fymin, fzmin, fxmax, fymax, fzmax);
      // support only one range - so add only if was not set before
      if (range.Size(0) == 0) range.AddRange(0,fxmin,fxmax);
      if (range.Size(1) == 0) range.AddRange(1,fymin,fymax);
      if (range.Size(2) == 0) range.AddRange(2,fzmin,fzmax);
   }
#ifdef DEBUG
   printf("range  size %d\n", range.Size(0) ); 
   if (range.Size(0)) {
      double x1; double x2; range.GetRange(0,x1,x2); 
      printf(" range in x = [%f,%f] \n",x1,x2);
   }
#endif

   // fill data  
   std::auto_ptr<ROOT::Fit::BinData> fitdata(new ROOT::Fit::BinData(opt,range) );
   ROOT::Fit::FillData(*fitdata, h1, f1); 
   if (fitdata->Size() == 0 ) { 
      Warning("Fit","Fit data is empty ");
      return -1;
   }

#ifdef DEBUG
   printf("HFit:: data size is %d \n",fitdata->Size());
   for (unsigned int i = 0; i < fitdata->Size(); ++i) { 
      if (fitdata->NDim() == 1) printf(" x[%d] = %f - value = %f \n", i,*(fitdata->Coords(i)),fitdata->Value(i) ); 
   }
#endif   

   // switch off linear fitting in case data has coordinate errors and the option is set
   if (fitdata->GetErrorType() == ROOT::Fit::BinData::kCoordError && fitdata->Opt().fCoordErrors ) linear = false;
   // linear fit cannot be done also in case of asymmetric errors 
   if (fitdata->GetErrorType() == ROOT::Fit::BinData::kAsymError && fitdata->Opt().fAsymErrors ) linear = false;

   // this functions use the TVirtualFitter
   if (special != 0 && !fitOption.Bound && !linear) { 
      if      (special == 100)      ROOT::Fit::InitGaus  (*fitdata,f1); // gaussian
      else if (special == 110)      ROOT::Fit::Init2DGaus(*fitdata,f1); // 2D gaussian
      else if (special == 400)      ROOT::Fit::InitGaus  (*fitdata,f1); // landau (use the same)
      else if (special == 410)      ROOT::Fit::Init2DGaus(*fitdata,f1); // 2D landau (use the same)

      else if (special == 200)      ROOT::Fit::InitExpo  (*fitdata, f1); // exponential

   }


   // set the fit function 
   // if option grad is specified use gradient 
   if ( (linear || fitOption.Gradient) )  
      fitter->SetFunction(ROOT::Math::WrappedMultiTF1(*f1) );
   else 
      fitter->SetFunction(static_cast<const ROOT::Math::IParamMultiFunction &>(ROOT::Math::WrappedMultiTF1(*f1) ) );

   // error normalization in case of zero error in the data
   if (fitdata->GetErrorType() == ROOT::Fit::BinData::kNoError) fitConfig.SetNormErrors(true);
   // normalize errors also in case you are fitting a Ndim histo with a N-1 function
   if (int(fitdata->NDim())  == hdim -1 ) fitConfig.SetNormErrors(true);

   
   // here need to get some static extra information (like max iterations, error def, etc...)


   // parameter settings and transfer the parameters values, names and limits from the functions
   // is done automatically in the Fitter.cxx 
   for (int i = 0; i < npar; ++i) { 
      ROOT::Fit::ParameterSettings & parSettings = fitConfig.ParSettings(i); 

      // check limits
      double plow,pup; 
      f1->GetParLimits(i,plow,pup);  
      if (plow*pup != 0 && plow >= pup) { // this is a limitation - cannot fix a parameter to zero value
         parSettings.Fix();
      }
      else if (plow < pup ) {
         if (!TMath::Finite(pup) && TMath::Finite(plow) ) 
            parSettings.SetLowerLimit(plow);
         else if (!TMath::Finite(plow) && TMath::Finite(pup) ) 
            parSettings.SetUpperLimit(pup);
         else 
            parSettings.SetLimits(plow,pup);
      }

      // set the parameter step size (by default are set to 0.3 of value)
      // if function provides meaningful error values
      double err = f1->GetParError(i); 
      if ( err > 0) 
         parSettings.SetStepSize(err); 
      else if (plow < pup && TMath::Finite(plow) && TMath::Finite(pup) ) { // in case of limits improve step sizes 
         double step = 0.1 * (pup - plow); 
         // check if value is not too close to limit otherwise trim value
         if (  parSettings.Value() < pup && pup - parSettings.Value() < 2 * step  ) 
            step = (pup - parSettings.Value() ) / 2; 
         else if ( parSettings.Value() > plow && parSettings.Value() - plow < 2 * step ) 
            step = (parSettings.Value() - plow ) / 2; 
         
         parSettings.SetStepSize(step); 
      }
      
      
   }

   // needed for setting precision ? 
   //   - Compute sum of squares of errors in the bin range
   // should maybe use stat[1] ??
 //   Double_t ey, sumw2=0;
//    for (i=hxfirst;i<=hxlast;i++) {
//       ey = GetBinError(i);
//       sumw2 += ey*ey;
//    }


   // set all default minimizer options (tolerance, max iterations, etc..)
   fitConfig.SetMinimizerOptions(minOption); 

   // specific  print level options 
   if (fitOption.Verbose) fitConfig.MinimizerOptions().SetPrintLevel(3); 
   if (fitOption.Quiet)    fitConfig.MinimizerOptions().SetPrintLevel(0); 

   // specific minimizer options depending on minimizer 
   if (linear) { 
      if (fitOption.Robust  ) { 
         // robust fitting
         std::string type = "Robust";
         // if an h is specified print out the value adding to the type 
         if (fitOption.hRobust > 0 && fitOption.hRobust < 1.)
            type += " (h=" + ROOT::Math::Util::ToString(fitOption.hRobust) + ")";
         fitConfig.SetMinimizer("Linear",type.c_str());
         fitConfig.MinimizerOptions().SetTolerance(fitOption.hRobust); // use tolerance for passing robust parameter
      }
      else 
         fitConfig.SetMinimizer("Linear","");
   }
   else { 
      if (fitOption.More) fitConfig.SetMinimizer("Minuit","MigradImproved");
   }


   // check if Error option (run Hesse and Minos) then 
   if (fitOption.Errors) { 
      // run Hesse and Minos
      fitConfig.SetParabErrors(true);
      fitConfig.SetMinosErrors(true);
   }


   // do fitting 

#ifdef DEBUG
   if (fitOption.Like)   printf("do  likelihood fit...\n");
   if (linear)    printf("do linear fit...\n");
   printf("do now  fit...\n");
#endif   
 
   bool fitok = false; 


   // check if can use option user 
   //typedef  void (* MinuitFCN_t )(int &npar, double *gin, double &f, double *u, int flag);
   TVirtualFitter::FCNFunc_t  userFcn = 0;  
   if (fitOption.User && TVirtualFitter::GetFitter() ) { 
      userFcn = (TVirtualFitter::GetFitter())->GetFCN(); 
      (TVirtualFitter::GetFitter())->SetUserFunc(f1); 
   }
   

   if (fitOption.User && userFcn) // user provided fit objective function
      fitok = fitter->FitFCN( userFcn );
   else if (fitOption.Like)  {// likelihood fit 
      // perform a weighted likelihood fit by applying weight correction to errors
      bool weight = ((fitOption.Like & 2) == 2);
      fitConfig.SetWeightCorrection(weight);
      bool extended = ((fitOption.Like & 4 ) != 4 );
      //if (!extended) Info("HFitImpl","Do a not -extended binned fit");
      fitok = fitter->LikelihoodFit(*fitdata, extended);
   }
   else // standard least square fit
      fitok = fitter->Fit(*fitdata); 


   if ( !fitok  && !fitOption.Quiet )
      Warning("Fit","Abnormal termination of minimization.");
   iret |= !fitok; 
        

   const ROOT::Fit::FitResult & fitResult = fitter->Result(); 
   // one could set directly the fit result in TF1
   iret = fitResult.Status(); 
   if (!fitResult.IsEmpty() ) { 
      // set in f1 the result of the fit      
      f1->SetChisquare(fitResult.Chi2() );
      f1->SetNDF(fitResult.Ndf() );
      f1->SetNumberFitPoints(fitdata->Size() );

      f1->SetParameters( &(fitResult.Parameters().front()) ); 
      if ( int( fitResult.Errors().size()) >= f1->GetNpar() ) 
         f1->SetParErrors( &(fitResult.Errors().front()) ); 
  
   }

//   - Store fitted function in histogram functions list and draw
      if (!fitOption.Nostore) {
         HFit::GetDrawingRange(h1, range);
         HFit::StoreAndDrawFitFunction(h1, f1, range, !fitOption.Plus, !fitOption.Nograph, goption); 
      }

      // print the result 
      // if using Fitter class must be done here 
      // use old style Minuit for TMinuit and if no corrections have been applied 
      if (!fitOption.Quiet) { 
         if (fitter->GetMinimizer() && fitConfig.MinimizerType() == "Minuit" && 
             !fitConfig.NormalizeErrors() && fitOption.Like <= 1) { 
            fitter->GetMinimizer()->PrintResults(); // use old style Minuit
         }
         else { 
            // print using FitResult class
            if (fitOption.Verbose) fitResult.PrintCovMatrix(std::cout); 
            fitResult.Print(std::cout);
         }
      }


      // store result in the backward compatible VirtualFitter
      TVirtualFitter * lastFitter = TVirtualFitter::GetFitter(); 
      // pass ownership of Fitter and Fitdata to TBackCompFitter (fitter pointer cannot be used afterwards)
      // need to get the raw pointer due to the  missing template copy ctor of auto_ptr on solaris
      // reset fitdata(cannot use anymore , ownership is passed)
      TBackCompFitter * bcfitter = new TBackCompFitter(fitter, std::auto_ptr<ROOT::Fit::FitData>(fitdata.release()));
      bcfitter->SetFitOption(fitOption); 
      bcfitter->SetObjectFit(h1);
      bcfitter->SetUserFunc(f1);
      bcfitter->SetBit(TBackCompFitter::kCanDeleteLast);
      if (userFcn) { 
         bcfitter->SetFCN(userFcn); 
         // for interpreted FCN functions
         if (lastFitter->GetMethodCall() ) bcfitter->SetMethodCall(lastFitter->GetMethodCall() );
      }
         
      // delete last fitter if it has been created here before
      if (lastFitter) {          
         TBackCompFitter * lastBCFitter = dynamic_cast<TBackCompFitter *> (lastFitter); 
         if (lastBCFitter && lastBCFitter->TestBit(TBackCompFitter::kCanDeleteLast) ) 
            delete lastBCFitter; 
      }
      //N.B=  this might create a memory leak if user does not delete the fitter he creates
      TVirtualFitter::SetFitter( bcfitter ); 

      // use old-style for printing the results
      // if (fitOption.Verbose) bcfitter->PrintResults(2,0.);
      // else if (!fitOption.Quiet) bcfitter->PrintResults(1,0.);

      if (fitOption.StoreResult) 
      {
         TFitResult* fr = new TFitResult(fitResult);
         TString name = "TFitResult-";
         name = name + h1->GetName() + "-" + f1->GetName();
         TString title = "TFitResult-";
         title += h1->GetTitle();
         fr->SetName(name);
         fr->SetTitle(title);
         return TFitResultPtr(fr);
      }
      else 
         return TFitResultPtr(iret);
}


void HFit::GetDrawingRange(TH1 * h1, ROOT::Fit::DataRange & range) { 
   // get range from histogram and update the DataRange class  
   // if a ranges already exist in that dimension use that one

   Int_t ndim = GetDimension(h1);

   double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0; 
   if (range.Size(0) == 0) { 
      TAxis  & xaxis = *(h1->GetXaxis()); 
      Int_t hxfirst = xaxis.GetFirst();
      Int_t hxlast  = xaxis.GetLast();
      Double_t binwidx = xaxis.GetBinWidth(hxlast);
      xmin    = xaxis.GetBinLowEdge(hxfirst);
      xmax    = xaxis.GetBinLowEdge(hxlast) +binwidx;
      range.AddRange(xmin,xmax);
   } 

   if (ndim > 1) {
      if (range.Size(1) == 0) { 
         TAxis  & yaxis = *(h1->GetYaxis()); 
         Int_t hyfirst = yaxis.GetFirst();
         Int_t hylast  = yaxis.GetLast();
         Double_t binwidy = yaxis.GetBinWidth(hylast);
         ymin    = yaxis.GetBinLowEdge(hyfirst);
         ymax    = yaxis.GetBinLowEdge(hylast) +binwidy;
         range.AddRange(1,ymin,ymax);
      }
   }      
   if (ndim > 2) {
      if (range.Size(2) == 0) { 
         TAxis  & zaxis = *(h1->GetZaxis()); 
         Int_t hzfirst = zaxis.GetFirst();
         Int_t hzlast  = zaxis.GetLast();
         Double_t binwidz = zaxis.GetBinWidth(hzlast);
         zmin    = zaxis.GetBinLowEdge(hzfirst);
         zmax    = zaxis.GetBinLowEdge(hzlast) +binwidz;
         range.AddRange(2,zmin,zmax);
      }
   }      
#ifdef DEBUG
   std::cout << "xmin,xmax" << xmin << "  " << xmax << std::endl;
#endif

}

void HFit::GetDrawingRange(TGraph * gr,  ROOT::Fit::DataRange & range) { 
   // get range for graph (used sub-set histogram)
   // N.B. : this is different than in previous implementation of TGraph::Fit where range used was from xmin to xmax.
   TH1 * h1 = gr->GetHistogram();
   // an histogram is normally always returned for a TGraph
   if (h1) HFit::GetDrawingRange(h1, range);
}
void HFit::GetDrawingRange(TMultiGraph * mg,  ROOT::Fit::DataRange & range) { 
   // get range for multi-graph (used sub-set histogram)
   // N.B. : this is different than in previous implementation of TMultiGraph::Fit where range used was from data xmin to xmax.
   TH1 * h1 = mg->GetHistogram();
   if (h1) {
      HFit::GetDrawingRange(h1, range);
   } 
   else if (range.Size(0) == 0) { 
      // compute range from all the TGraph's belonging to the MultiGraph
      double xmin = std::numeric_limits<double>::infinity(); 
      double xmax = -std::numeric_limits<double>::infinity(); 
      TIter next(mg->GetListOfGraphs() );
      TGraph * g = 0; 
      while (  (g = (TGraph*) next() ) ) { 
         double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
         g->ComputeRange(x1,y1,x2,y2); 
         if (x1 < xmin) xmin = x1; 
         if (x2 > xmax) xmax = x2; 
      }
      range.AddRange(xmin,xmax);
   }
}
void HFit::GetDrawingRange(TGraph2D * gr,  ROOT::Fit::DataRange & range) { 
   // get range for graph2D (used sub-set histogram)
   // N.B. : this is different than in previous implementation of TGraph2D::Fit. There range used was always(0,0)
   TH1 * h1 = gr->GetHistogram();
   if (h1) HFit::GetDrawingRange(h1, range);
}

void HFit::GetDrawingRange(THnBase * s1, ROOT::Fit::DataRange & range) { 
   // get range from histogram and update the DataRange class  
   // if a ranges already exist in that dimension use that one

   Int_t ndim = GetDimension(s1);

   for ( int i = 0; i < ndim; ++i ) {
      if ( range.Size(i) == 0 ) {
         TAxis *axis = s1->GetAxis(i);
         range.AddRange(i, axis->GetXmin(), axis->GetXmax());
      }
   }
}

template<class FitObject>
void HFit::StoreAndDrawFitFunction(FitObject * h1, TF1 * f1, const ROOT::Fit::DataRange & range, bool delOldFunction, bool drawFunction, const char *goption) { 
//   - Store fitted function in histogram functions list and draw
// should have separate functions for 1,2,3d ? t.b.d in case

#ifdef DEBUG
   std::cout <<"draw and store fit function " << f1->GetName() << std::endl;
#endif
 

   Int_t ndim = GetDimension(h1);
   double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0; 
   if (range.Size(0) ) range.GetRange(0,xmin,xmax);
   if (range.Size(1) ) range.GetRange(1,ymin,ymax);
   if (range.Size(2) ) range.GetRange(2,zmin,zmax);


#ifdef DEBUG
   std::cout <<"draw and store fit function " << f1->GetName() 
             << " Range in x = [ " << xmin << " , " << xmax << " ]" << std::endl;
#endif

   TList * funcList = h1->GetListOfFunctions();
   if (funcList == 0){
      Error("StoreAndDrawFitFunction","Function list has not been created - cannot store the fitted function");
      return;
   } 

   // delete the function in the list only if 
   // the funciton we are fitting is not in that list
   // If this is the case we re-use that function object and 
   // we do not create a new one (if delOldFunction is true)
   bool reuseOldFunction = false; 
   if (delOldFunction) {
      TIter next(funcList, kIterBackward);
      TObject *obj;
      while ((obj = next())) {
         if (obj->InheritsFrom(TF1::Class())) {
            if (obj != f1) {
               funcList->Remove(obj);
               delete obj;
            }
            else {
               reuseOldFunction = true; 
            }
         }
      }
   }

   TF1 *fnew1 = 0;
   TF2 *fnew2 = 0;
   TF3 *fnew3 = 0;

   // copy TF1 using TClass to avoid slicing in case of derived classes
   if (ndim < 2) {
      if (!reuseOldFunction) { 
         fnew1 = (TF1*)f1->IsA()->New();
         R__ASSERT(fnew1);
         f1->Copy(*fnew1);
         funcList->Add(fnew1);
      }
      else { 
         fnew1 = f1;
      }
      fnew1->SetParent( h1 );
      fnew1->SetRange(xmin,xmax);
      fnew1->Save(xmin,xmax,0,0,0,0);
      if (!drawFunction) fnew1->SetBit(TF1::kNotDraw);
      fnew1->SetBit(TFormula::kNotGlobal);
   } else if (ndim < 3) {
      if (!reuseOldFunction) { 
         fnew2 = (TF2*)f1->IsA()->New();
         R__ASSERT(fnew2);
         f1->Copy(*fnew2);
         funcList->Add(fnew2);
      }
      else { 
         fnew2 = dynamic_cast<TF2*>(f1);
         R__ASSERT(fnew2);
      }
      fnew2->SetRange(xmin,ymin,xmax,ymax);
      fnew2->SetParent( h1 );
      fnew2->Save(xmin,xmax,ymin,ymax,0,0);
      if (!drawFunction) fnew2->SetBit(TF1::kNotDraw);
      fnew2->SetBit(TFormula::kNotGlobal);
   } else { 
      if (!reuseOldFunction) { 
         fnew3 = (TF3*)f1->IsA()->New();
         R__ASSERT(fnew3);
         f1->Copy(*fnew3);
         funcList->Add(fnew3);
      }
      else {
         fnew2 = dynamic_cast<TF3*>(f1);
         R__ASSERT(fnew3);         
      }
      fnew3->SetRange(xmin,ymin,zmin,xmax,ymax,zmax);
      fnew3->SetParent( h1 );
      fnew3->Save(xmin,xmax,ymin,ymax,zmin,zmax);
      if (!drawFunction) fnew3->SetBit(TF1::kNotDraw);
      fnew3->SetBit(TFormula::kNotGlobal);
   }
   if (h1->TestBit(kCanDelete)) return;
   // draw only in case of histograms
   if (drawFunction && ndim < 3 && h1->InheritsFrom(TH1::Class() ) ) h1->Draw(goption);
   if (gPad) gPad->Modified(); // this is not in TH1 code (needed ??)
   
   return; 
}


void ROOT::Fit::FitOptionsMake(EFitObjectType type, const char *option, Foption_t &fitOption) { 
   //   - Decode list of options into fitOption (used by both TGraph and TH1)
   //  works for both histograms and graph depending on the enum FitObjectType defined in HFit

   if (option == 0) return;
   if (strlen(option) == 0) return;
 
   TString opt = option;
   opt.ToUpper();

   // parse firt the specific options
   if (type == kHistogram) { 
      if (opt.Contains("I"))  fitOption.Integral= 1;   // integral of function in the bin (no sense for graph)
      if (opt.Contains("WW")) fitOption.W1      = 2; //all bins have weight=1, even empty bins
   }
   
   // specific Graph options (need to be parsed before)
   else if (type == kGraph) { 
      opt.ReplaceAll("ROB", "H");
      opt.ReplaceAll("EX0", "T");

      //for robust fitting, see if # of good points is defined
      // decode parameters for robust fitting
      Double_t h=0;
      if (opt.Contains("H=0.")) {
         int start = opt.Index("H=0.");
         int numpos = start + strlen("H=0.");
         int numlen = 0;
         int len = opt.Length();
         while( (numpos+numlen<len) && isdigit(opt[numpos+numlen]) ) numlen++;
         TString num = opt(numpos,numlen);
         opt.Remove(start+strlen("H"),strlen("=0.")+numlen);
         h = atof(num.Data());
         h*=TMath::Power(10, -numlen);
      }

      if (opt.Contains("H")) { fitOption.Robust  = 1;   fitOption.hRobust = h; } 
      if (opt.Contains("T")) fitOption.NoErrX   = 1;  // no error in X

   }

   if (opt.Contains("U")) fitOption.User    = 1;
   if (opt.Contains("W")) fitOption.W1     = 1; // all non-empty bins have weight =1
   if (opt.Contains("Q")) fitOption.Quiet   = 1;
   if (opt.Contains("V")){fitOption.Verbose = 1; fitOption.Quiet   = 0;}
   if (opt.Contains("L")) fitOption.Like    = 1;
   if (opt.Contains("X")) fitOption.Chi2    = 1;
   if (opt.Contains("P")) fitOption.PChi2    = 1; 

   // likelihood fit options
   if (opt.Contains("L")) { 
      fitOption.Like    = 1;
      //if (opt.Contains("LL")) fitOption.Like    = 2;
      if (opt.Contains("W")){ fitOption.Like    = 2;  fitOption.W1=0;}//  (weighted likelihood)
      if (opt.Contains("MULTI")) { 
         if (fitOption.Like == 2) fitOption.Like = 6; // weighted multinomial 
         else fitOption.Like    = 4; // multinomial likelihood fit instead of Poisson
         opt.ReplaceAll("MULTI","");
      }
   }
   if (opt.Contains("E")) fitOption.Errors  = 1;
   if (opt.Contains("R")) fitOption.Range   = 1;
   if (opt.Contains("G")) fitOption.Gradient= 1;
   if (opt.Contains("M")) fitOption.More    = 1;
   if (opt.Contains("N")) fitOption.Nostore = 1;
   if (opt.Contains("0")) fitOption.Nograph = 1;
   if (opt.Contains("+")) fitOption.Plus    = 1;
   if (opt.Contains("B")) fitOption.Bound   = 1;  
   if (opt.Contains("C")) fitOption.Nochisq = 1;
   if (opt.Contains("F")) fitOption.Minuit  = 1;
   if (opt.Contains("S")) fitOption.StoreResult   = 1;

}

void HFit::CheckGraphFitOptions(Foption_t & foption) { 
   if (foption.Like) { 
      Info("CheckGraphFitOptions","L (Log Likelihood fit) is an invalid option when fitting a graph. It is ignored");
      foption.Like = 0; 
   }
   if (foption.Integral) { 
      Info("CheckGraphFitOptions","I (use function integral) is an invalid option when fitting a graph. It is ignored");
      foption.Integral = 0; 
   }
   return;
} 

// implementation of unbin fit function (defined in HFitInterface)

TFitResultPtr ROOT::Fit::UnBinFit(ROOT::Fit::UnBinData * fitdata, TF1 * fitfunc, Foption_t & fitOption , const ROOT::Math::MinimizerOptions & minOption) { 
   // do unbin fit, ownership of fitdata is passed later to the TBackFitter class

#ifdef DEBUG
   printf("tree data size is %d \n",fitdata->Size());
   for (unsigned int i = 0; i < fitdata->Size(); ++i) { 
      if (fitdata->NDim() == 1) printf(" x[%d] = %f \n", i,*(fitdata->Coords(i) ) ); 
   }
#endif   
   if (fitdata->Size() == 0 ) { 
      Warning("Fit","Fit data is empty ");
      return -1;
   }
      
   // create the fitter
   std::auto_ptr<ROOT::Fit::Fitter> fitter(new ROOT::Fit::Fitter() );
   ROOT::Fit::FitConfig & fitConfig = fitter->Config();

   // dimension is given by data because TF1 pointer can have wrong one
   unsigned int dim = fitdata->NDim();    

   // set the fit function
   // if option grad is specified use gradient 
   // need to create a wrapper for an automatic  normalized TF1 ???
   if ( fitOption.Gradient ) {
      assert ( (int) dim == fitfunc->GetNdim() );
      fitter->SetFunction(ROOT::Math::WrappedTF1(*fitfunc) );
   }
   else 
      fitter->SetFunction(static_cast<const ROOT::Math::IParamMultiFunction &>(ROOT::Math::WrappedMultiTF1(*fitfunc, dim) ) );

   // parameter setting is done automaticaly in the Fitter class 
   // need only to set limits
   int npar = fitfunc->GetNpar();
   for (int i = 0; i < npar; ++i) { 
      ROOT::Fit::ParameterSettings & parSettings = fitConfig.ParSettings(i); 
      double plow,pup; 
      fitfunc->GetParLimits(i,plow,pup);  
      // this is a limitation of TF1 interface - cannot fix a parameter to zero value
      if (plow*pup != 0 && plow >= pup) {
         parSettings.Fix();
      }
      else if (plow < pup ) {
         if (!TMath::Finite(pup) && TMath::Finite(plow) ) 
            parSettings.SetLowerLimit(plow);
         else if (!TMath::Finite(plow) && TMath::Finite(pup) ) 
            parSettings.SetUpperLimit(pup);
         else 
            parSettings.SetLimits(plow,pup);
      }

      // set the parameter step size (by default are set to 0.3 of value)
      // if function provides meaningful error values
      double err = fitfunc->GetParError(i); 
      if ( err > 0) 
         parSettings.SetStepSize(err); 
      else if (plow < pup && TMath::Finite(plow) && TMath::Finite(pup) ) { // in case of limits improve step sizes 
         double step = 0.1 * (pup - plow); 
         // check if value is not too close to limit otherwise trim value
         if (  parSettings.Value() < pup && pup - parSettings.Value() < 2 * step  ) 
            step = (pup - parSettings.Value() ) / 2; 
         else if ( parSettings.Value() > plow && parSettings.Value() - plow < 2 * step ) 
            step = (parSettings.Value() - plow ) / 2; 
         
         parSettings.SetStepSize(step); 
      }

   }

   fitConfig.SetMinimizerOptions(minOption); 

   if (fitOption.Verbose)   fitConfig.MinimizerOptions().SetPrintLevel(3); 
   if (fitOption.Quiet)     fitConfig.MinimizerOptions().SetPrintLevel(0); 
  
   // more 
   if (fitOption.More)   fitConfig.SetMinimizer("Minuit","MigradImproved");

   // chech if Minos or more options
   if (fitOption.Errors) { 
      // run Hesse and Minos
      fitConfig.SetParabErrors(true);
      fitConfig.SetMinosErrors(true);
   }
   // use weight correction
   if ( (fitOption.Like & 2) == 2) 
      fitConfig.SetWeightCorrection(true);

   bool extended = (fitOption.Like & 1) == 1;
   
   bool fitok = false; 
   fitok = fitter->Fit(*fitdata, extended); 
   if ( !fitok  && !fitOption.Quiet )
      Warning("UnBinFit","Abnormal termination of minimization.");

   const ROOT::Fit::FitResult & fitResult = fitter->Result(); 
   // one could set directly the fit result in TF1
   int iret = fitResult.Status(); 
   if (!fitResult.IsEmpty() ) { 
      // set in fitfunc the result of the fit      
      fitfunc->SetNDF(fitResult.Ndf() );
      fitfunc->SetNumberFitPoints(fitdata->Size() );

      fitfunc->SetParameters( &(fitResult.Parameters().front()) ); 
      if ( int( fitResult.Errors().size()) >= fitfunc->GetNpar() ) 
         fitfunc->SetParErrors( &(fitResult.Errors().front()) ); 
  
   }

   // store result in the backward compatible VirtualFitter
   TVirtualFitter * lastFitter = TVirtualFitter::GetFitter(); 
   // pass ownership of Fitter and Fitdata to TBackCompFitter (fitter pointer cannot be used afterwards)
   TBackCompFitter * bcfitter = new TBackCompFitter(fitter, std::auto_ptr<ROOT::Fit::FitData>(fitdata));
 // cannot use anymore now fitdata (given away ownership)
   fitdata = 0;
   bcfitter->SetFitOption(fitOption); 
   //bcfitter->SetObjectFit(fTree);
   bcfitter->SetUserFunc(fitfunc);
   
   if (lastFitter) delete lastFitter; 
   TVirtualFitter::SetFitter( bcfitter ); 
   
   // print results
//       if (!fitOption.Quiet) fitResult.Print(std::cout);
//       if (fitOption.Verbose) fitResult.PrintCovMatrix(std::cout); 
   
   // use old-style for printing the results
   if (fitOption.Verbose) bcfitter->PrintResults(2,0.);
   else if (!fitOption.Quiet) bcfitter->PrintResults(1,0.);

   if (fitOption.StoreResult) 
   {
      TFitResult* fr = new TFitResult(fitResult);
      TString name = "TFitResult-";
      name = name + "UnBinData-" + fitfunc->GetName();
      TString title = "TFitResult-";
      title += name;
      fr->SetName(name);
      fr->SetTitle(title);
      return TFitResultPtr(fr);
   }
   else 
      return TFitResultPtr(iret);
}


// implementations of ROOT::Fit::FitObject functions (defined in HFitInterface) in terms of the template HFit::Fit

TFitResultPtr ROOT::Fit::FitObject(TH1 * h1, TF1 *f1 , Foption_t & foption , const ROOT::Math::MinimizerOptions &
moption, const char *goption, ROOT::Fit::DataRange & range) { 
   // check fit options
   // check if have weights in case of weighted likelihood
   if ( ((foption.Like & 2) == 2) && h1->GetSumw2N() == 0) { 
      Warning("HFit::FitObject","A weighted likelihood fit is requested but histogram is not weighted - do a standard Likelihood fit");
      foption.Like = 1;
   }
   // histogram fitting
   return HFit::Fit(h1,f1,foption,moption,goption,range); 
}

TFitResultPtr ROOT::Fit::FitObject(TGraph * gr, TF1 *f1 , Foption_t & foption , const ROOT::Math::MinimizerOptions & moption, const char *goption, ROOT::Fit::DataRange & range) { 
  // exclude options not valid for graphs
   HFit::CheckGraphFitOptions(foption);
    // TGraph fitting
   return HFit::Fit(gr,f1,foption,moption,goption,range); 
}

TFitResultPtr ROOT::Fit::FitObject(TMultiGraph * gr, TF1 *f1 , Foption_t & foption , const ROOT::Math::MinimizerOptions & moption, const char *goption, ROOT::Fit::DataRange & range) { 
  // exclude options not valid for graphs
   HFit::CheckGraphFitOptions(foption);
    // TMultiGraph fitting
   return HFit::Fit(gr,f1,foption,moption,goption,range); 
}

TFitResultPtr ROOT::Fit::FitObject(TGraph2D * gr, TF1 *f1 , Foption_t & foption , const ROOT::Math::MinimizerOptions & moption, const char *goption, ROOT::Fit::DataRange & range) { 
  // exclude options not valid for graphs
   HFit::CheckGraphFitOptions(foption);
    // TGraph2D fitting
   return HFit::Fit(gr,f1,foption,moption,goption,range); 
}

TFitResultPtr ROOT::Fit::FitObject(THnBase * s1, TF1 *f1 , Foption_t & foption , const ROOT::Math::MinimizerOptions & moption, const char *goption, ROOT::Fit::DataRange & range) { 
   // sparse histogram fitting
   return HFit::Fit(s1,f1,foption,moption,goption,range); 
}



// Int_t TGraph2D::DoFit(TF2 *f2 ,Option_t *option ,Option_t *goption) { 
//    // internal graph2D fitting methods
//    Foption_t fitOption;
//    ROOT::Fit::FitOptionsMake(option,fitOption);

//    // create range and minimizer options with default values 
//    ROOT::Fit::DataRange range(2); 
//    ROOT::Math::MinimizerOptions minOption; 
//    return ROOT::Fit::FitObject(this, f2 , fitOption , minOption, goption, range); 
// }

