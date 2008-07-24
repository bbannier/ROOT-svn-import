// new TH1Fit function 
//______________________________________________________________________________


#include "TH1.h"
#include "TF1.h"
#include "TError.h"

#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "HFitInterface.h"

#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"

#include "TList.h"
#include "TF2.h"
#include "TF3.h"

#include "TClass.h"

#include <cmath>

//#define DEBUG

// utility functions used in TH1::Fit

namespace TH1Fit { 

   Int_t CheckFitFunction(const TH1 * h1, const TF1 * f1);

   void DrawFitFunction(TH1 * h1, const TF1 * f1, const ROOT::Fit::DataRange & range, bool, bool, Option_t *goption);
   
   Int_t Fit(TH1 * h1, TF1 *f1 , Foption_t & option ,Option_t *goption, Double_t xxmin, Double_t xxmax); 
} 

Int_t TH1Fit::CheckFitFunction(const TH1 * h1, const TF1 * f1) { 
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
   if (f1->GetNdim() > h1->GetDimension()) {
      Error("Fit","function %s dimension, %d, is greater than histogram dimension, %d",
            f1->GetName(), f1->GetNdim(), h1->GetDimension());
      return -4;
   }
   // t.b.d what to do if dimension is less 

   return 0; 

}


Int_t TH1Fit::Fit(TH1 * h1, TF1 *f1 , Foption_t & fitOption ,Option_t *goption, Double_t xxmin, Double_t xxmax)
{
   // perform fit using new fitting classes 

#ifdef DEBUG
   printf("fit function %s\n",f1->GetName() ); 
#endif

   // replacement function using  new fitter
   int iret = TH1Fit::CheckFitFunction(h1,f1);
   if (iret != 0) return iret; 



   // why this ???? (special care should be taken for functions with less dimension than histograms)
   if (f1->GetNdim() < h1->GetDimension() ) fitOption.Integral = 0; 

   Int_t special = f1->GetNumber();
   Bool_t linear = f1->IsLinear();
   Int_t npar = f1->GetNpar();
   if (special==299+npar)
      linear = kTRUE;
   if (fitOption.Bound || fitOption.Like || fitOption.Errors || fitOption.Gradient || fitOption.More || fitOption.User|| fitOption.Integral || fitOption.Minuit)
      linear = kFALSE;


   // create the fitter
   std::auto_ptr<ROOT::Fit::Fitter> fitter = std::auto_ptr<ROOT::Fit::Fitter> (new ROOT::Fit::Fitter() );
   ROOT::Fit::FitConfig & fitConfig = fitter->Config();

   // create options 
   ROOT::Fit::DataOptions opt; 
   opt.fIntegral = fitOption.Integral; 
   opt.fUseRange = fitOption.Range; 


   // check if a range is given or is in the function
   ROOT::Fit::DataRange range; 
   if (xxmin != xxmax) {
      range.AddRange(xxmin,xxmax,0);
   }


   if (opt.fUseRange) { 
#ifdef DEBUG
      printf("use range \n" ); 
#endif
      // check if function has range 
      Double_t fxmin, fymin, fzmin, fxmax, fymax, fzmax;
      f1->GetRange(fxmin, fymin, fzmin, fxmax, fymax, fzmax);
      // support only one range interval in x ? 
      if (fxmin < fxmax) range.AddRange(fxmin, fxmax,0);
      if (fymin < fymax) range.AddRange(fymin, fymax,1);
      if (fzmin < fzmax) range.AddRange(fzmin, fzmax,2);
   }
#ifdef DEBUG
   printf("range  size %d\n", range.Size(0) ); 
#endif

   // fill data  
   ROOT::Fit::BinData fitdata(opt,range);
   ROOT::Fit::FillData(fitdata, h1, f1); 

#ifdef DEBUG
   printf("TH1Fit:: data size is %d \n",fitdata.Size());
#endif   

   // this functions use the TVirtualFitter
   if (special != 0 && !fitOption.Bound && !linear) { 
      if      (special == 100)      ROOT::Fit::InitGaus(fitdata,f1);  // gaussian
      else if (special == 400)      ROOT::Fit::InitGaus(fitdata,f1);  // landau (use the same)
   // need to do a linear fit first for expo and poly ? 
//             else if (special == 200)      H1InitExpo();
//             else if (special == 299+npar) H1InitPolynom();
   }


   // set the fit function 
   // if option grad is specified use gradient (works only for 1D) 
   if ( (linear || fitOption.Gradient)  && f1->GetNdim() == 1) 
      fitter->SetFunction(ROOT::Math::WrappedTF1(*f1) );
   else 
      fitter->SetFunction(ROOT::Math::WrappedMultiTF1(*f1) );




   if (!fitOption.Verbose) fitConfig.MinimizerOptions().SetPrintLevel(0); 
   else fitConfig.MinimizerOptions().SetPrintLevel(3); 

   
   // here need to get some static extra information (like max iterations, error def, etc...)

   // something in case of gradient functions

   // parameter settings and transfer the parameters values, names and limits from the functions
   // is done automatically in the Fitter.cxx 
   for (int i = 0; i < npar; ++i) { 
      ROOT::Fit::ParameterSettings & parSettings = fitConfig.ParSettings(i); 
      double plow,pup; 
      f1->GetParLimits(i,plow,pup);  
      if (plow*pup != 0 && plow >= pup) { // this is a limitation - cannot fix a parameter to zero value
         parSettings.Fix();
      }
      else if (plow < pup ) 
         parSettings.SetLimits(plow,pup);
   }

   // needed for setting precision ? 
   //   - Compute sum of squares of errors in the bin range
   // should maybe use stat[1] ??
 //   Double_t ey, sumw2=0;
//    for (i=hxfirst;i<=hxlast;i++) {
//       ey = GetBinError(i);
//       sumw2 += ey*ey;
//    }

   // set some minimizer options
   if (linear) fitConfig.SetMinimizer("Linear");
   if (fitOption.More) fitConfig.SetMinimizer("Minuit","MigradImproved");
   //fitConfig.MinimizerOptions().SetTolerance(sumw2*1.);

   // do fitting 

#ifdef DEBUG
   printf("do now fit...\n");
#endif   
 
   bool fitok = false; 

   if (fitOption.Like)
      fitok = fitter->LikelihoodFit(fitdata);
   else 
      fitok = fitter->Fit(fitdata); 


   if ( !fitok  && !fitOption.Quiet )
      Warning("Fit","Abnormal termination of minimization.");
   iret |= !fitok; 
    
   // chech if Minos or more options
   if (fitOption.Errors) { 
      // run Hesse and Minos
      fitConfig.MinimizerOptions().SetParabErrors(true);
      fitConfig.MinimizerOptions().SetMinosErrors(true);
   }
    

   const ROOT::Fit::FitResult & fitResult = fitter->Result(); 
   // one could set directly the fit result in TF1
   //  if (fitResult.IsValid() ) { 
      // set in f1 the result of the fit      
      f1->SetChisquare(fitResult.Chi2() );
      f1->SetNDF(fitResult.Ndf() );

      f1->SetParameters( &(fitResult.Parameters().front()) ); 
      f1->SetParErrors( &(fitResult.Errors().front()) ); 
  
      // print results
      if (!fitOption.Quiet) fitResult.Print(std::cout); 
//   }
//    else {
//       iret = -1; 
//    }

//   - Store fitted function in histogram functions list and draw
      if (!fitOption.Nostore) 
         TH1Fit::DrawFitFunction(h1, f1, range, !fitOption.Plus, !fitOption.Nograph, goption); 

      return iret; 
}

void TH1Fit::DrawFitFunction(TH1 * h1, const TF1 * f1, const ROOT::Fit::DataRange & range, bool delOldFunction, bool drawFunction, Option_t *goption) { 
//   - Store fitted function in histogram functions list and draw
// should have separate functions for 1,2,3d ? t.b.d in case

 
   TF1 *fnew1;
   TF2 *fnew2;
   TF3 *fnew3;

   Int_t ndim = h1->GetDimension();

   // get range for the function 
   // support so far only one range / coordinate
   double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0; 
   if (range.Size(0) == 0) { 
      TAxis  & xaxis = *(h1->GetXaxis()); 
      Int_t hxfirst = xaxis.GetFirst();
      Int_t hxlast  = xaxis.GetLast();
      Double_t binwidx = xaxis.GetBinWidth(hxlast);
      xmin    = xaxis.GetBinLowEdge(hxfirst);
      xmax    = xaxis.GetBinLowEdge(hxlast) +binwidx;
   } else  { 
      range.GetRange(0,xmin,xmax);
   }
   if (ndim > 1) {
      if (range.Size(1) == 0) { 
         TAxis  & yaxis = *(h1->GetYaxis()); 
         Int_t hyfirst = yaxis.GetFirst();
         Int_t hylast  = yaxis.GetLast();
         Double_t binwidy = yaxis.GetBinWidth(hylast);
         ymin    = yaxis.GetBinLowEdge(hyfirst);
         ymax    = yaxis.GetBinLowEdge(hylast) +binwidy;
      } else  { 
         range.GetRange(1,ymin,ymax);
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
      } else  { 
         range.GetRange(2,zmin,zmax);
      }
   }      

   TList * fFunctions = h1->GetListOfFunctions();


   if (delOldFunction) {
      TIter next(fFunctions, kIterBackward);
      TObject *obj;
      while ((obj = next())) {
         if (obj->InheritsFrom(TF1::Class())) {
            fFunctions->Remove(obj);
            delete obj;
         }
      }
   }


   if (ndim < 2) {
      fnew1 = (TF1*)f1->IsA()->New();
      f1->Copy(*fnew1);
      fFunctions->Add(fnew1);
      fnew1->SetParent( h1 );
      fnew1->Save(xmin,xmax,0,0,0,0);
      if (!drawFunction) fnew1->SetBit(TF1::kNotDraw);
      fnew1->SetBit(TFormula::kNotGlobal);
   } else if (ndim < 3) {
      fnew2 = (TF2*)f1->IsA()->New();
      f1->Copy(*fnew2);
      fnew2 = (TF2*)f1->Clone();
      fFunctions->Add(fnew2);
      fnew2->SetParent( h1 );
      fnew2->Save(xmin,xmax,ymin,ymax,0,0);
      if (!drawFunction) fnew2->SetBit(TF1::kNotDraw);
      fnew2->SetBit(TFormula::kNotGlobal);
   } else {
      // 3D- why f3d is not saved ???
      fnew3 = (TF3*)f1->IsA()->New();
      f1->Copy(*fnew3);
      fnew3 = (TF3*)f1->Clone();
      fFunctions->Add(fnew3);
      fnew3->SetParent( h1 );
      fnew3->SetBit(TFormula::kNotGlobal);
   }
   if (h1->TestBit(kCanDelete)) return;
   if (drawFunction && ndim < 3) h1->Draw(goption);
   
   return; 
}

Int_t TH1::DoFit(TF1 *f1 ,Option_t *option ,Option_t *goption, Double_t xxmin, Double_t xxmax) { 
//   - Decode list of options into fitOption
   Foption_t fitOption;
   if (!FitOptionsMake(option,fitOption)) return 0;
   return TH1Fit::Fit(this, f1 , fitOption , goption, xxmin, xxmax); 
}
