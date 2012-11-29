#include "Math/Functor.h"
#include <iostream>
 
class MyFunction {
 public:
  double operator()(const double* x) const {
    cout<<"Hello.\n";
    return x[0]+x[1];
   }
};
 
int function(){
   // test directly calling the function object
   MyFunction myf;
   
   // test from function object
   ROOT::Math::Functor f1(myf,2);
 
   double x[] = {1,2};
 
   cout << f1(x) << endl;
   
   return 0;
}
