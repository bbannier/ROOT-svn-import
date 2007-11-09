// @(#)root/mathmore:$Id: LinkDef_SpecFunc.h 19826 2007-09-19 19:56:11Z rdm $
// Authors: L. Moneta, A. Zsenei   08/2005 

#ifdef __CINT__

// define header gurad symbols to avoid CINT re-including the file 
#pragma link C++ global ROOT_Math_SpecFuncMathMore;
#pragma link C++ global ROOT_Math_PdfFuncMathMore;
#pragma link C++ global ROOT_Math_QuantFuncMathMore;

// special functions

#pragma link C++ function ROOT::Math::assoc_laguerre(unsigned,double,double);
#pragma link C++ function ROOT::Math::assoc_legendre(unsigned,unsigned,double);
#pragma link C++ function ROOT::Math::comp_ellint_1(double);
#pragma link C++ function ROOT::Math::comp_ellint_2(double);
#pragma link C++ function ROOT::Math::comp_ellint_3(double, double);
#pragma link C++ function ROOT::Math::conf_hyperg(double,double,double);
#pragma link C++ function ROOT::Math::conf_hypergU(double,double,double);
#pragma link C++ function ROOT::Math::cyl_bessel_i(double,double);
#pragma link C++ function ROOT::Math::cyl_bessel_j(double,double);
#pragma link C++ function ROOT::Math::cyl_bessel_k(double,double);
#pragma link C++ function ROOT::Math::cyl_neumann(double,double);
#pragma link C++ function ROOT::Math::ellint_1(double,double);
#pragma link C++ function ROOT::Math::ellint_2(double,double);
#pragma link C++ function ROOT::Math::ellint_3(double,double,double);
#pragma link C++ function ROOT::Math::expint(double);
#pragma link C++ function ROOT::Math::hyperg(double,double,double,double);
#pragma link C++ function ROOT::Math::laguerre(unsigned,double);
#pragma link C++ function ROOT::Math::legendre(unsigned,double);
#pragma link C++ function ROOT::Math::riemann_zeta(double);
#pragma link C++ function ROOT::Math::sph_bessel(unsigned,double);
#pragma link C++ function ROOT::Math::sph_legendre(unsigned,unsigned,double);
#pragma link C++ function ROOT::Math::sph_neumann(unsigned,double);

// statistical functions: pdf of mathmore
#pragma link C++ function ROOT::Math::beta_pdf( double , double, double);
#pragma link C++ function ROOT::Math::landau_pdf( double , double, double);

// quantiles: inverses of cdf
#pragma link C++ function ROOT::Math::breitwigner_quantile_c(double,double);
#pragma link C++ function ROOT::Math::breitwigner_quantile(double,double);
#pragma link C++ function ROOT::Math::cauchy_quantile_c(double,double);
#pragma link C++ function ROOT::Math::cauchy_quantile(double,double);
#pragma link C++ function ROOT::Math::chisquared_quantile_c(double,double);
#pragma link C++ function ROOT::Math::chisquared_quantile(double,double);
#pragma link C++ function ROOT::Math::exponential_quantile_c(double,double);
#pragma link C++ function ROOT::Math::exponential_quantile(double,double);
#pragma link C++ function ROOT::Math::gamma_quantile_c(double,double,double);
#pragma link C++ function ROOT::Math::gamma_quantile(double,double,double);
#pragma link C++ function ROOT::Math::gaussian_quantile_c(double,double);
#pragma link C++ function ROOT::Math::gaussian_quantile(double,double);
#pragma link C++ function ROOT::Math::lognormal_quantile_c(double,double,double);
#pragma link C++ function ROOT::Math::lognormal_quantile(double,double,double);
#pragma link C++ function ROOT::Math::normal_quantile_c(double,double);
#pragma link C++ function ROOT::Math::normal_quantile(double,double);
#pragma link C++ function ROOT::Math::tdistribution_quantile_c(double,double);
#pragma link C++ function ROOT::Math::tdistribution_quantile(double,double);
#pragma link C++ function ROOT::Math::uniform_quantile_c(double,double,double);
#pragma link C++ function ROOT::Math::uniform_quantile(double,double,double);


#endif
