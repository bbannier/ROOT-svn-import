#include <iostream>
#include "TFn.h"

int main(int argc, char* argv[]) {

   Double_t min[1] = { 0.5 };
   Double_t max[1] = { 4.0 };

   TFn f("square", "1/x", min, max);

   std::cout << "GetMinimum  " << f.GetMinimum()  << std::endl;
   std::cout << "GetMaximum  " << f.GetMaximum()  << std::endl;
   std::cout << "GetMinimumX " << f.GetMinimumX()[0] << std::endl;
   std::cout << "GetMaximumX " << f.GetMaximumX()[0] << std::endl;


   return 0;
}


