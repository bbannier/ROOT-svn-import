#include <iostream>
#include <iomanip>
#include <cmath>
#include "TFn.h"
#include "TF2.h"
#include "THn.h"
#include "Math/ParamFunctor.h"
#include "Math/DistSampler.h"
#include "TFoamSampler.h"
#include "/home/ibucur/rootdev/trunk/hist/hist/src/TFn.cxx"

#define DIM 5

// first test function - PASSED
Double_t sum(Double_t *x, Double_t *) {
   return exp(x[0]) + log(x[1]) - x[2] + x[3] * x[3] - x[4];
}

Double_t sum1(Double_t *x, Double_t *) {
   return x[0];
}

class CintClass {
public:
   Double_t CintEval(Double_t *x, Double_t*) {
      return x[0] + sin(x[1]);
   }
};

int cint_func() {
   

   Double_t min[DIM] = { 0.0, 1.0, -2.0, 2.0, 0.0 };
   Double_t max[DIM] = { 1.0, 4.0, 2.0, 3.0, 1.0 };

   CintClass *cl = new CintClass;

   TFn fi("FiveDimCint", 3, cl, &CintClass::CintEval, min, max, 0, "CintClass", "CintEval");
   TFn fa = fi;
   TFn fc(fi);

   std::cout << "Cint Function minmax orig " << fi.GetMinimum() << " " << fi.GetMaximum() << " " << fi.Integral() << std::endl;
   std::cout << "Cint Function minmax asgn " << fa.GetMinimum() << " " << fa.GetMaximum() << " " << fa.Integral() << std::endl;
   std::cout << "Cint Function minmax copy " << fc.GetMinimum() << " " << fc.GetMaximum() << " " << fc.Integral() << std::endl;

   return 0;
}
// second test function

int main(int argc, char* argv[]) {

   // TODO: solve overload issue
   Double_t min0 = 0.0;
   Double_t max0 = 1.0;
   Double_t min2[2] = {0.0, -2.0};
   Double_t max2[2] = {3.0, 2.0};


   // FIRST SET OF TESTS
   Double_t min[DIM] = { 0.0, 1.0, -2.0, 2.0, 0.0 };
   Double_t max[DIM] = { 1.0, 4.0, 2.0, 3.0, 1.0 };
   TFn fsum("FiveDimTFn", DIM, &sum, min, max);
   TFn fsum_copy(fsum);
   TFn fsum_copy_assign = fsum_copy; 
   std::cout << "FIRST SET OF TESTS "; fsum_copy_assign.Print();
   std::cout << "The underlying mathematical function that describes this object is 'exp(x[0]) + log(x[1]) - x[2] + x[3] * x[3] - x[4]'" << std::endl;
   std::cout << "The final object used for this test is actually derived from an original object, by first applying the copy constructor, and then the assignment operator. This way, we are testing both the basic specification of the TFn class, as well as the essential C++ copy functionality." << std::endl << std::endl;

   std::cout << "Normalisation constant (::Integral over full domain): " << fsum_copy_assign.Integral() << std::endl;
   std::cout << "Function global minimum (::GetMinimum): " << fsum_copy_assign.GetMinimum(NULL, NULL, 1e-6, 1000)  << std::endl;
   std::cout << "Function global maximum (::GetMaximum): " << fsum_copy_assign.GetMaximum(NULL, NULL, 1e-6, 1000)  << std::endl;

   Double_t *minX = fsum_copy_assign.GetMinimumX(NULL, NULL, 1e-6, 1000);
   Double_t *maxX = fsum_copy_assign.GetMaximumX(NULL, NULL, 1e-6, 1000);
   std::cout << "Coordinates of global minimum (::GetMinimumX): "; for(UInt_t i = 0; i < DIM; ++i) std::cout << minX[i] << " "; std::cout << std::endl;
   std::cout << "Coordinates of global maximum (::GetMaximumX): "; for(UInt_t i = 0; i < DIM; ++i) std::cout << maxX[i] << " "; std::cout << std::endl;
   std::cout << "Partial derivatives at range lower bound: "; for(UInt_t i = 0; i < DIM; ++i) std::cout << fsum_copy_assign.Derivative(min, i) << " "; 
   std::cout << std::endl;
   std::cout << "Partial derivatives at range upper bound: "; for(UInt_t i = 0; i < DIM; ++i) std::cout << fsum_copy_assign.Derivative(max, i) << " "; 
   std::cout << std::endl;

   TF1* fsum_proj1d = fsum_copy_assign.Projection1D(0);
   std::cout << "1D Projection: "; fsum_proj1d->Print();
   Double_t x = 0; std::cout << "TF1 proj1d: proj1d(0.0) = " << (*fsum_proj1d)(&x);
   x = 0.5; std::cout << "; proj1d(0.5) = " << (*fsum_proj1d)(&x);
   x = 1.0; std::cout << "; proj1d(1.0) = " << (*fsum_proj1d)(&x) << std::endl;

   TF1 f1("OneDimTFn", &sum1, 2.0, 3.0);
   TFn fn("FiveDim", "x[0] + x[1] + x[2] + x[3] + x[4]", min, max);
   TFn fn11("OneDimOnePar", "[0] * exp(x)", &min0, &max0);
   TFn f2("TwoDim", "x[0]^2 + x[1]", min2, max2);
   TFn_Distribution tnd(f2);


//   cint_func();

   std::cout << "GetHistogram: " << std::endl;
   UInt_t npoints[DIM] = { 3, 2, 1, 2, 1 };
   THn* histo = fn.GetHistogram(npoints);
   std::cout << "32121   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 1; std::cout << "12121   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 2; std::cout << "22121   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[1] = 1; std::cout << "21121   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[3] = 1; std::cout << "21111   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 1; std::cout << "11111   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 0; std::cout << "01111   " << histo->GetBinContent((const Int_t *)npoints) << std::endl;

   std::cout << "TFn_Distribution values: " << std::endl;
   std::cout << "min " << tnd(min2) << " max " << tnd(max2) << std::endl;


   for(Int_t i = 0; i < 30; ++i) {
      const Double_t* x = f2.GetRandom();
      for(Int_t j = 0; j < 2; ++j) std::cout << x[j] << " ";
      std::cout << std::endl;
   }

   return 0;
}


