#include <iostream>
#include "TFn.h"

#define DIM 2

int main(int argc, char* argv[]) {

   Double_t min[DIM] = { 0.0, 0.0 };
   Double_t max[DIM] = { 1.0, 1.0 };

   TFn f("square", "x[0]*x[0]+x[1]", min, max);

   std::cout << "GetMinimum  " << f.GetMinimum()  << std::endl;
   std::cout << "GetMaximum  " << f.GetMaximum()  << std::endl;

   Double_t *minX = f.GetMinimumX();
   Double_t *maxX = f.GetMinimumX();
   
   std::cout << "GetMinimumX ";
   for(int i = 0; i < DIM; ++i) std::cout << minX[i] << " ";
   std::cout << std::endl;

   std::cout << "GetMaximumX ";
   for(int i = 0; i < DIM; ++i) std::cout << maxX[i] << " ";
   std::cout << std::endl;


   return 0;
}


