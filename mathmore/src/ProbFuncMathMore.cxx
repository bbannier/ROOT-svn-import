// @(#)root/mathmore:$Id$
// Authors: L. Moneta, A. Zsenei   08/2005 


#include <cmath>
#include "Math/ProbFuncMathMore.h"
#include "Math/SpecFuncMathCore.h"
#include "gsl/gsl_cdf.h"


namespace ROOT {
namespace Math {

  


  
  double fdistribution_cdf_c(double x, double n, double m, double x0) {

    return ROOT::Math::inc_beta(m/(m + n*(x-x0)), .5*m, .5*n);
  
  }


   double fdistribution_cdf(double x, double n, double m, double x0) {

    return ROOT::Math::inc_beta(n*(x-x0)/(m + n*(x-x0)), .5*n, .5*m);
  }



  double gamma_cdf_c(double x, double alpha, double theta, double x0) {

     return 1. - ROOT::Math::inc_gamma(alpha, (x-x0)/theta);

  }



  double gamma_cdf(double x, double alpha, double theta, double x0) {

      return ROOT::Math::inc_gamma(alpha, (x-x0)/theta);
  }





  double tdistribution_cdf_c(double x, double r, double x0) {

    double p = x-x0;
    double sign = (p>0) ? 1. : -1;
    return .5 - .5*ROOT::Math::inc_beta(p*p/(r + p*p), .5, .5*r)*sign;

  }



  double tdistribution_cdf(double x, double r, double x0) {

    double p = x-x0;
    double sign = (p>0) ? 1. : -1;
    return  .5 + .5*ROOT::Math::inc_beta(p*p/(r + p*p), .5, .5*r)*sign;

  }


  double beta_cdf_c(double x, double a, double b) {

    return 1. - ROOT::Math::inc_beta(x, a, b);

  }


  double beta_cdf(double x, double a, double b ) {

    return ROOT::Math::inc_beta(x, a, b);

  }

   double poisson_cdf_c(unsigned int n, double mu) {

          return gsl_cdf_poisson_Q(n, mu);

   }

   double poisson_cdf(unsigned int n, double mu) {

          return gsl_cdf_poisson_P(n, mu);

   }

   double binomial_cdf_c(unsigned int k, double p, unsigned int n) {

          return gsl_cdf_binomial_Q(k, p, n);

   }

   double binomial_cdf(unsigned int k, double p, unsigned int n) {

          return gsl_cdf_binomial_P(k, p, n);

   }

} // namespace Math
} // namespace ROOT



