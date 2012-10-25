#include <iostream>
#include <iomanip>
#include <cmath>
#include "TFn.h"
#include "TF2.h"
#include "Math/ParamFunctor.h"

#define DIM 5

// first test function - PASSED
Double_t sum(Double_t *x, Double_t *) {
   return x[0] * x[0] * x[0] + x[1] - std::exp(x[2]) + std::log(x[3]) - x[4];
}

// second test function

int main(int argc, char* argv[]) {

   Double_t min[DIM] = { 1.0, 1.0, 1.0, 2.0, 0.0 };
   Double_t max[DIM] = { 2.0, 2.0, 2.0, 7.0, 5.0 };

   TFn f("FiveDimTFn", DIM, &sum, min, max, 0);
//   TF2 f2("TF2", "x[0] + 2 * x[1]", 0.0, 1.0, 0.0, 1.0);

  

   std::cout << "GetMinimum  " << f.GetMinimum(NULL, NULL, 1e-6, 1000)  << std::endl;
   std::cout << "GetMaximum  " << f.GetMaximum(NULL, NULL, 1e-6, 1000)  << std::endl;


   Double_t *minX = f.GetMinimumX(NULL, NULL, 1e-6, 1000);
   Double_t *maxX = f.GetMaximumX(NULL, NULL, 1e-6, 1000);
   
   std::cout << "GetMinimumX ";
   for(int i = 0; i < DIM; ++i) std::cout << minX[i] << " ";
   std::cout << std::endl;
   

   std::cout << "GetMaximumX ";
   for(int i = 0; i < DIM; ++i) std::cout << maxX[i] << " ";
   std::cout << std::endl;

   std::cout << std::setprecision(15) << "P.deriv.coord.0: " << f.Derivative(min, 0) << " " << f.Derivative(max, 0) << std::endl;
   std::cout << "P.deriv.coord.1: " << f.Derivative(min, 2) << " " << f.Derivative(max, 2) << std::endl;

   return 0;
}


