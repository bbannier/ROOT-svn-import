#include <iostream>
#include <iomanip>
#include <cmath>
#include "TFn.h"
#include "TF2.h"
#include "THn.h"
#include "Math/ParamFunctor.h"
#include "Math/DistSampler.h"
#include "TFoamSampler.h"

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

   Double_t min[DIM] = { 0.0, 1.0, -2.0, 2.0, 0.0 };
   Double_t max[DIM] = { 1.0, 4.0, 2.0, 3.0, 1.0 };
   Double_t min0 = 0.0;
   Double_t max0 = 1.0;

//   TF1 f1("OneDimFn", "x + 2", 0.0, 1.0);
   // TODO: solve overload problem
   TFn fx("FiveDimTFn", DIM, &sum, min, max);
   TFn f = fx ; 
   TF1 f1("OneDimTFn", &sum1, 2.0, 3.0);
   TFn fn("FiveDim", "x[0] + x[1] + x[2] + x[3] + x[4]", min, max);
   TFn fn11("OneDimOnePar", "[0] * exp(x)", &min0, &max0);
 
   std::cout << "f object name is " << f.GetName() << std::endl;
   std::cout << "Normalisation constant " << f.Integral() << std::endl;
   std::cout << "GetMinimum  " << f.GetMinimum(NULL, NULL, 1e-6, 1000)  << std::endl;
   std::cout << "GetMaximum  " << f.GetMaximum(NULL, NULL, 1e-6, 1000)  << std::endl;
   std::cout << "GetMinimum fx " << fx.GetMinimum(NULL, NULL, 1e-6, 1000)  << std::endl;
   std::cout << "GetMaximum fx " << f.GetMaximum(NULL, NULL, 1e-6, 1000)  << std::endl;


   Double_t *minX = f.GetMinimumX(NULL, NULL, 1e-6, 1000);
   Double_t *maxX = f.GetMaximumX(NULL, NULL, 1e-6, 1000);
   
   std::cout << "GetMinimumX ";
   for(int i = 0; i < DIM; ++i) std::cout << minX[i] << " ";
   std::cout << std::endl;
   

   std::cout << "GetMaximumX ";
   for(int i = 0; i < DIM; ++i) std::cout << maxX[i] << " ";
   std::cout << std::endl;

   for(Int_t i = 0; i < DIM; ++i) 
      std::cout << std::setprecision(6) << "P.deriv.coord" << i <<": " << f.Derivative(min, i) << " " << f.Derivative(max, i) << std::endl;

   TF1* f1_c0 = f.Projection1D(0);

   Double_t x = 0;

   std::cout << "TF1 coord 0: (0) = " << (*f1_c0)(&x);
   x = 0.5; std::cout << "; (1/2) = " << (*f1_c0)(&x);
   x = 1.0; std::cout << "; (1.0) = " << (*f1_c0)(&x) << std::endl;

   cint_func();

   UInt_t npoints[DIM] = { 10, 1, 1, 1, 1 };
   THn* histo = fn.GetHistogram(npoints);
   histo->Print("v");

   for(Int_t i = 0; i < 10; ++i) {
      const Double_t* x = fn.GetRandom();
      for(Int_t j = 0; j < DIM; ++j) std::cout << x[j] << " ";
      std::cout << std::endl;
   }

   return 0;
}


