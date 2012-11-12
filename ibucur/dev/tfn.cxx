#include <iostream>
#include <iomanip>
#include <cmath>
#include "TFn.h"
#include "TF2.h"
#include "THn.h"
#include "Math/ParamFunctor.h"
#include "Math/DistSampler.h"
#include "TFoamSampler.h"

#ifndef __CINT__
#include "/home/ibucur/rootdev/trunk/hist/hist/src/TFn.cxx"
#endif


#define DIM1 5
#define DIM2 2

// first test function - PASSED
Double_t sum(Double_t *x, Double_t *) {
   return exp(x[0]) + log(x[1]) - x[2] + x[3] * x[3] - x[4];
}

class CintClass {
public:
   Double_t CintEval(Double_t *x, Double_t*) {
      return x[0] + sin(x[1]);
   }
};

int cint_func() {
   Double_t min[DIM2] = { 0.0, 1.0};
   Double_t max[DIM2] = { 1.0, 4.0};
   
   CintClass* cint_class = new CintClass();

   TFn fcint("TwoDimCint", DIM2, cint_class, &CintClass::CintEval, min, max, 0, "CintClass", "CintEval");
   std::cout << "FOURTH SET OF TESTS "; fcint.Print();
   std::cout << "The underlying mathematical function that describes this object is 'x[0] + sin(x[1])'" << std::endl << std::endl;

   std::cout << "Normalisation constant (::Integral over full domain): " << fcint.Integral() << std::endl;
   std::cout << "Function global minimum (::GetMinimum): " << fcint.GetMinimum()  << std::endl;
   std::cout << "Function global maximum (::GetMaximum): " << fcint.GetMaximum()  << std::endl;
   std::cout << std::endl << std::endl << std::endl;

   return 0;
}
// second test function


int main(int argc, char* argv[]) {

   // TODO: solve overload issue

   // FIRST SET OF TESTS
   Double_t min[DIM1] = { 0.0, 1.0, -2.0, 2.0, 0.0 };
   Double_t max[DIM1] = { 1.0, 4.0, 2.0, 3.0, 1.0 };
   TFn fsum("FiveDimTFn", DIM1, &sum, min, max);
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
   std::cout << "Coordinates of global minimum (::GetMinimumX): "; for(UInt_t i = 0; i < DIM1; ++i) std::cout << minX[i] << " "; std::cout << std::endl;
   std::cout << "Coordinates of global maximum (::GetMaximumX): "; for(UInt_t i = 0; i < DIM1; ++i) std::cout << maxX[i] << " "; std::cout << std::endl;
   std::cout << "Partial derivatives at range lower bound: "; for(UInt_t i = 0; i < DIM1; ++i) std::cout << fsum_copy_assign.Derivative(min, i) << " "; 
   std::cout << std::endl;
   std::cout << "Partial derivatives at range upper bound: "; for(UInt_t i = 0; i < DIM1; ++i) std::cout << fsum_copy_assign.Derivative(max, i) << " "; 
   std::cout << std::endl;

   TF1* fsum_proj1d = fsum_copy_assign.Projection1D(0);
   std::cout << "1D Projection: "; fsum_proj1d->Print();
   Double_t x = 0; std::cout << "TF1 proj1d: proj1d(0.0) = " << (*fsum_proj1d)(&x);
   x = 0.5; std::cout << "; proj1d(0.5) = " << (*fsum_proj1d)(&x);
   x = 1.0; std::cout << "; proj1d(1.0) = " << (*fsum_proj1d)(&x) << std::endl;
   std::cout << std::endl << std::endl << std::endl;

   // SECOND SET OF TESTS
   TFn flin("FiveLinearDim", "x[0] + x[1] + x[2] + x[3] + x[4]", min, max);
   std::cout << "SECOND SET OF TESTS "; flin.Print();
   std::cout << "The underlying mathematical function that describes this object is 'x[0] + x[1] + x[2] + x[3] + x[4]'" << std::endl << std::endl;

   UInt_t npoints[DIM1] = { 3, 2, 1, 2, 1 };
   THn* histo = flin.GetHistogram(npoints);
   std::cout << "::GetHistogram with the following number of points/dimension - (3,2,1,2,1) :" << std::endl;
                   std::cout << "hist(3,2,1,2,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 1; std::cout << "hist(1,2,1,2,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 2; std::cout << "hist(2,2,1,2,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[1] = 1; std::cout << "hist(2,1,1,2,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[3] = 1; std::cout << "hist(2,1,1,1,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 1; std::cout << "hist(1,1,1,1,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   npoints[0] = 0; std::cout << "hist(0,1,1,1,1) = " << histo->GetBinContent((const Int_t *)npoints) << std::endl;
   std::cout << std::endl << std::endl << std::endl;

   // THIRD SET OF TESTS
   Double_t min2[DIM2] = {0.0, -2.0};
   Double_t max2[DIM2] = {3.0, 2.0};
   Double_t avg2[DIM2] = {1.0, 0.0};
   TFn f2("TwoDim", "x[0]^2 + x[1]", min2, max2);
   std::cout << "THIRD SET OF TESTS "; f2.Print();
   std::cout << "The underlying mathematical function that describes this object is 'x[0]^2 + x[1]'" << std::endl << std::endl;

   TFn_Distribution f2_dist(f2);
   std::cout << "TFn_Distribution obtained from this TFn has values: " << std::endl;
   std::cout << "TFn_Distribution(0, -2) = " << f2_dist(min2) << std::endl;
   std::cout << "TFn_Distribution(1,  0) = " << f2_dist(avg2) << std::endl;
   std::cout << "TFn_Distribution(3,  2) = " << f2_dist(max2) << std::endl;

   std::cout << "Random samples extracted from the TFn distribution: " << std::endl;
   for(Int_t i = 0; i < 30; ++i) {
      std::cout << "Sample " << i << ": ";
      const Double_t* x = f2.GetRandom();
      for(Int_t j = 0; j < DIM2; ++j) std::cout << x[j] << " ";
      std::cout << std::endl;
   }
   std::cout << std::endl << std::endl << std::endl;


   // FOURTH SET OF TESTS
   cint_func();


   return 0;
}


