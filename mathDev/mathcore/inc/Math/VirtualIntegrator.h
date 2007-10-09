// Interface for numerical integration
// in one and many dimensions

#ifndef ROOT_Math_VirtualIntegrator
#define ROOT_Math_VirtualIntegrator

#ifndef ROOT_Math_IFunctionfwd
#include "Math/IFunctionfwd.h"
#endif



namespace ROOT {
namespace Math {


class VirtualIntegrator{
   public:

  //1-dim integration
  virtual double Integrate(double, double);


  //multi-dim integration
  virtual double Integrate(unsigned int dim, const double*, const double*);

  //setting a 1-dim function
  virtual void SetFunction(const IGenFunction &f);

 //setting a multi-dim function
  virtual void SetFunction(const IMultiGenFunction &f);

};


}//namespace Math
}//namespace ROOT

#endif /* ROOT_Math_VirtualIntegrator */
