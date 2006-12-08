
/* MathCore/tests/test_SpecFunc.cpp
 * 
 * Copyright (C) 2004 Andras Zsenei
 * 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/**

Test file for the special functions implemented in MathMore. For
the moment nothing exceptional. Evaluates the functions and checks
if the value is right against values copied from the GSL tests.

*/





#include <iostream> 
#include <iomanip> 
#include <string>

#include "Math/SpecFuncMathMore.h"
#ifndef NO_MATHCORE
#include "Math/SpecFuncMathCore.h"
#endif

#ifdef CHECK_WITH_GSL
#include <gsl/gsl_sf.h>
#endif

#ifndef PI
#define PI       3.14159265358979323846264338328      /* pi */
#endif


int compare( const std::string & name, double v1, double v2, double scale = 2.0) {
  //  ntest = ntest + 1; 

   std::cout << std::setw(50) << std::left << name << ":\t";   
   
  // numerical double limit for epsilon 
   double eps = scale* std::numeric_limits<double>::epsilon();
   int iret = 0; 
   double delta = v2 - v1;
   double d = 0;
   if (delta < 0 ) delta = - delta; 
   if (v1 == 0 || v2 == 0) { 
      if  (delta > eps ) { 
         iret = 1; 
      }
   }
   // skip case v1 or v2 is infinity
   else { 
      d = v1; 

      if ( v1 < 0) d = -d; 
      // add also case when delta is small by default
      if ( delta/d  > eps && delta > eps ) 
         iret =  1; 
   }

   if (iret == 0) 
      std::cout <<" OK" << std::endl;
   else { 
      std::cout <<" FAILED " << std::endl;
      int pr = std::cout.precision (18);
      std::cout << "\nDiscrepancy in " << name << "() :\n  " << v1 << " != " << v2 << " discr = " << int(delta/d/eps) 
                << "   (Allowed discrepancy is " << eps  << ")\n\n";
      std::cout.precision (pr);
      //nfail = nfail + 1;
   }
   return iret; 
}



// void showDiff(std::string name, double calculatedValue, double expectedValue, double scale = 1.0) {

//    compare(calculatedValue, expectedValue, name, scale)

//   std::cout << name << calculatedValue << " expected value: " << expectedValue;
//   int prec = std::cout.precision();
//   std::cout.precision(5);
//   std::cout << " diff: " << (calculatedValue-expectedValue) << " reldiff: " << 
//     (calculatedValue-expectedValue)/expectedValue << std::endl;
//   std::cout.precision(prec);

// }



int testSpecFunc() { 

   using namespace ROOT::Math;

   int iret = 0; 

   std::cout.precision(20);

//#ifndef NO_MATHCORE 
   // explicit put namespace to be sure to use right ones

  iret |= compare("tgamma(9.0) ", ROOT::Math::tgamma(9.0), 40320.0, 4);

  iret |= compare("erf(0.5) ", ROOT::Math::erf(0.5), 0.5204998778130465377);

  iret |= compare("erfc(-1.0) ", ROOT::Math::erfc(-1.0), 1.8427007929497148693);

 iret |= compare("beta(1.0, 5.0) ", ROOT::Math::beta(1.0, 5.0), 0.2);

//#endif

   iret |= compare("assoc_laguerre(4,  2, 0.5) ", assoc_laguerre(4, 2, 0.5), 6.752604166666666667);

   iret |= compare("assoc_legendre(10, 1, -0.5) ", assoc_legendre(10, 1, -0.5), -2.0066877394361256516);

   iret |= compare("comp_ellint_1(0.50) ", comp_ellint_1(0.50), 1.6857503548125960429);

   iret |= compare("comp_ellint_2(0.50) ", comp_ellint_2(0.50), 1.4674622093394271555);

   iret |= compare("comp_ellint_3(0.5, 0.5) ", comp_ellint_3(0.5, 0.5), 2.41367150420119, 16);

   iret |= compare("conf_hyperg(1, 1.5, 1) ", conf_hyperg(1, 1.5, 1), 2.0300784692787049755);

   iret |= compare("cyl_bessel_i(1.0, 1.0) ", cyl_bessel_i(1.0, 1.0), 0.5651591039924850272);

   iret |= compare("cyl_bessel_j(0.75, 1.0) ", cyl_bessel_j(0.75, 1.0), 0.5586524932048917478, 16);

   iret |= compare("cyl_bessel_k(1.0, 1.0) ", cyl_bessel_k(1.0, 1.0), 0.6019072301972345747);

   iret |= compare("cyl_neumann(0.75, 1.0) ", cyl_neumann(0.75, 1.0), -0.6218694174429746383 );

   iret |= compare("ellint_1(0.50, PI/3.0) ", ellint_1(0.50, PI/3.0), 1.0895506700518854093);

   iret |= compare("ellint_2(0.50, PI/3.0) ", ellint_2(0.50, PI/3.0), 1.0075555551444720293);

   iret |= compare("ellint_3(-0.50, 0.5, PI/3.0) ", ellint_3(-0.50, 0.5, PI/3.0), 0.9570574331323584890);

   iret |= compare("expint(1.0) ", expint(1.0), 1.8951178163559367555);

   // std::cout << "Hermite polynomials: to do!" << std::endl;

   iret |= compare("hyperg(8, -8, 1, 0.5) ", hyperg(8, -8, 1, 0.5), 0.13671875);

   //std::cout << "Laguerre polynomials: to do!" << std::endl;

   iret |= compare("legendre(10, -0.5) ", legendre(10, -0.5), -0.1882286071777345);

   iret |= compare("riemann_zeta(-0.5) ", riemann_zeta(-0.5), -0.207886224977354566017307, 16);

   iret |= compare("sph_bessel(1, 10.0) ", sph_bessel(1, 10.0), 0.07846694179875154709000);

   // std::cout << "Spherical associated Legendre functions: to do!" << std::endl;

   iret |= compare("sph_neumann(0, 1.0) ", sph_neumann(0, 1.0), -0.54030230586813972);


   if (iret != 0) { 
      std::cout << "\n\nError:  Special Functions Test FAILED !!!!!" << std::endl;
   }
   return iret; 

}

void getGSLErrors() { 

#ifdef CHECK_WITH_GSL
   gsl_sf_result r; 
   int iret; 

   iret = gsl_sf_ellint_P_e(PI/2.0, 0.5, -0.5, GSL_PREC_DOUBLE, &r);
   std::cout << "comp_ellint_3(0.50, 0.5) : " << r.val << " err:  " << r.err << "  iret:  " << iret << std::endl; 

   iret = gsl_sf_ellint_P_e(PI/3.0, 0.5, 0.5, GSL_PREC_DOUBLE, &r);
   std::cout << "ellint_3(0.50, 0.5, PI/3.0) : " << r.val << " err:  " << r.err << "  iret:  " << iret << std::endl; 
   
   iret = gsl_sf_zeta_e(-0.5, &r);
   std::cout << "riemann_zeta(-0.5) : " << r.val << " err:  " << r.err << "  iret:  " << iret << std::endl; 
#endif


}


int main() { 

   int iret = 0; 
   iret |=  testSpecFunc();

   getGSLErrors(); 
   return iret; 
}


