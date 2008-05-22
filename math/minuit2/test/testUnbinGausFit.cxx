#include "TMath.h"
#include "TSystem.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TROOT.h"

#include "Fit/UnBinData.h"
#include "Fit/Fitter.h"


#include "Math/IParamFunction.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Math/WrappedParamFunction.h"
#include "Math/MultiDimParamFunctionAdapter.h"

#include "TGraphErrors.h"

#include "TStyle.h"


#include "Math/DistFunc.h"

#include <string>
#include <iostream>

#include "TStopwatch.h"

#define DEBUG

// test fit with many dimension

const int N = 1; // 1d fit
const int NPar = 5; // sum of two gaussians 
const std::string branchType = "x[10]/D";
const int NPoints = 100000;
double truePar[NPar]; 
double iniPar[NPar]; 
const int nfit = 1;
const int strategy = 1;

double gausnorm(const double *x, const double *p) { 

   double invsig = 1./p[1]; 
   double tmp = (x[0]-p[0]) * invsig; 
   const double sqrt_2pi = 1./std::sqrt(2.* 3.14159 );
   return std::exp(-0.5 * tmp*tmp ) * sqrt_2pi * invsig; 
}

double gausSum(const double *x, const double *p) { 

   double f = p[0] * gausnorm(x,p+1) + (1.-p[0] ) * gausnorm(x, p+ 3);

   return f; 
}

struct MINUIT2 {
   static std::string name() { return "Minuit2"; }
   static std::string name2() { return ""; }
};

// fill fit data structure 
ROOT::Fit::UnBinData * FillUnBinData(TTree * tree ) { 

   // fill the unbin data set from a TTree
   ROOT::Fit::UnBinData * d = 0; 

   // for the large tree access directly the branches 
   d = new ROOT::Fit::UnBinData();

   unsigned int n = tree->GetEntries(); 
#ifdef DEBUG
   std::cout << "number of unbin data is " << n << " of dim " << N << std::endl;
#endif
   d->Initialize(n,N);
   TBranch * bx = tree->GetBranch("x"); 
   double vx[N];
   bx->SetAddress(vx); 
   std::vector<double>  m(N);
   for (int unsigned i = 0; i < n; ++i) {
     bx->GetEntry(i);
     d->Add(vx);
     for (int j = 0; j < N; ++j) 
       m[j] += vx[j];
   }

#ifdef DEBUG
   std::cout << "average values of means :\n"; 
   for (int j = 0; j < N; ++j) 
     std::cout << m[j]/n << "  ";
   std::cout << "\n";
#endif
   
   return d; 
}


// unbin fit

typedef ROOT::Math::IParamMultiFunction Func;  
template <class MinType, class T>
int DoUnBinFit(T * tree, Func & func, bool debug = false ) {  

   ROOT::Fit::UnBinData * d  = FillUnBinData(tree );  
   // need to have done Tree->Draw() before fit
   //FillUnBinData(d,tree);

   std::cout << "Filled the fit data " << std::endl;
   //printData(d);

#ifdef DEBUG
   std::cout << "data size type and size  is " << typeid(*d).name() <<  "   " << d->Size() << std::endl;
#endif
         
         

   // create the fitter 
   //std::cout << "Fit parameter 2  " << f.Parameters()[2] << std::endl;

   ROOT::Fit::Fitter fitter; 
   fitter.Config().SetMinimizer(MinType::name(),MinType::name2());

   if (debug) 
      fitter.Config().MinimizerOptions().SetPrintLevel(3);

   // set tolerance 1 for tree to be same as in TTTreePlayer::UnBinFIt
   fitter.Config().MinimizerOptions().SetTolerance(1);

   // set strategy (0 to avoid MnHesse
   //   fitter.Config().MinimizerOptions().SetStrategy(strategy);


   // create the function

   fitter.SetFunction(func); 
   // need to set limits to constant term
   fitter.Config().ParSettings(0).SetLimits(0.,1.);

   if (debug) 
     std::cout << "do fitting... " << std::endl;

   bool ret = fitter.Fit(*d);
   if (!ret) {
      std::cout << " Fit Failed " << std::endl;
      return -1; 
   }
   if (debug) 
      fitter.Result().Print(std::cout);    

   // check fit result
   double chi2 = 0;
   if (fitter.Result().Value(0) <  0.5 ) { 
     for (int i = 0; i < N; ++i) { 
       double d = (truePar[i] - fitter.Result().Value(i) )/ (fitter.Result().Error(i) );
       chi2 += d*d; 
     }
   }
   else { 
     double truePar2[NPar];
     truePar2[0] = 1.-truePar[0];
     truePar2[1] = truePar[3];
     truePar2[2] = truePar[4];
     truePar2[3] = truePar[1];
     truePar2[4] = truePar[2];
     for (int i = 0; i < N; ++i) { 
       double d = ( truePar2[i] - fitter.Result().Value(i) )/ (fitter.Result().Error(i) );
       chi2 += d*d; 
     }
   }
   double prob = ROOT::Math::chisquared_cdf_c(chi2,N);
   int iret =  (prob < 1.0E-6) ? -1 : 0;
   if (iret != 0) {
      std::cout <<"Found difference in fitted values - prob = " << prob << std::endl;
      fitter.Result().Print(std::cout);    
   }

   delete d; 

   return iret; 

}


template <class MinType>
int DoFit(TTree * tree, Func & func, bool debug = false ) {  
   return DoUnBinFit<MinType, TTree>(tree, func, debug); 
}
// template <class MinType>
// int DoFit(TH1 * h1, Func & func, bool debug = false, bool copyData = false ) {  
//    return DoBinFit<MinType, TH1>(h1, func, debug, copyData); 
// }
// template <class MinType>
// int DoFit(TGraph * gr, Func & func, bool debug = false, bool copyData = false ) {  
//    return DoBinFit<MinType, TGraph>(gr, func, debug, copyData); 
// }

template <class MinType, class FitObj>
int FitUsingNewFitter(FitObj * fitobj, Func & func ) { 

   std::cout << "\n************************************************************\n"; 
   std::cout << "\tFit using new Fit::Fitter  " << typeid(*fitobj).name() << std::endl;
   std::cout << "\tMinimizer is " << MinType::name() << "  " << MinType::name2() << " func dim = " << func.NDim() << std::endl; 

   int iret = 0; 
   TStopwatch w; w.Start(); 

#ifdef DEBUG
   std::cout << "initial Parameters " << iniPar << "  " << *iniPar << "   " <<  *(iniPar+1) << std::endl;
   func.SetParameters(iniPar);
   iret |= DoFit<MinType>(fitobj,func,true );
   if (iret != 0) {
     std::cout << "Fit failed " << std::endl;
   }

#else
   for (int i = 0; i < nfit; ++i) { 
      func.SetParameters(iniPar);
      iret = DoFit<MinType>(fitobj,func, false);
      if (iret != 0) {
         std::cout << "Fit failed " << std::endl;
         break; 
      }
   }
#endif
   w.Stop(); 
   std::cout << "\nTime: \t" << w.RealTime() << " , " << w.CpuTime() << std::endl;  
   std::cout << "\n************************************************************\n"; 

   return iret; 
}


int testNdimFit() { 


   std::cout << "\n\n************************************************************\n"; 
   std::cout << "\t UNBINNED TREE (GAUSSIAN MULTI-DIM)  FIT\n";
   std::cout << "************************************************************\n"; 

   TTree t1("t2","a large Tree with gaussian variables");
   double  x[N];
   Int_t ev;
   t1.Branch("x",x,branchType.c_str());
   t1.Branch("ev",&ev,"ev/I");

   // generate the true parameters 
//       for (int j = 0;  j < NGaus; ++j) {   
// 	 double a = j+1;
//          double mu = double(j)/NGaus; 
//          double s  = 1.0 + double(j)/NGaus;  
//          truePar[3*j] = a; 
//          truePar[3*j+1] = mu; 
//          truePar[3*j+2] = s;
// 	 tot += a;
//       }
   truePar[0] = 0.2; // first gaussian
   truePar[1] = 1.;
   truePar[2] = 2; 
   truePar[3] = -1; 
   truePar[4] = 5; 

      
   
   //fill the tree
   TRandom3 r; 
   for (Int_t i=0;i<NPoints;i++) {
      for (int j = 0;  j < N; ++j) { 
	if (r.Rndm() < truePar[0] ) { 
	  double mu = truePar[1]; 
	  double s  = truePar[2];  
	  x[j] = r.Gaus(mu,s);
	}
	else { 
	  double mu = truePar[3]; 
	  double s  = truePar[4];  
	  x[j] = r.Gaus(mu,s);
	}
      }

      ev = i;
      t1.Fill();
      
   }
   //t1.Draw("x"); // to select fit variable 

   iniPar[0] = 0.5; 
   for (int i = 0; i <2; ++i) {
      iniPar[2*i+1] = 0; 
      iniPar[2*i+2] = 1; 
   }

   // use simply TF1 wrapper 
   //ROOT::Math::WrappedMultiTF1 f2(*f1); 
   ROOT::Math::WrappedParamFunction<> f2(&gausSum,1,5,iniPar); 

   int iret = 0; 
   iret |= FitUsingNewFitter<MINUIT2>(&t1,f2);

   return iret; 
}

int main() { 
  return testNdimFit();
}
