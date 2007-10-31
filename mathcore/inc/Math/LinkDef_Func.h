// @(#)root/mathcore:$Id$
// Authors: Andras Zsenei & Lorenzo Moneta   06/2005 

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/
#ifdef __CINT__


#pragma link C++ typedef ROOT::Math::IGenFunction;
#pragma link C++ typedef ROOT::Math::IMultiGenFunction;
#pragma link C++ typedef ROOT::Math::IGradFunction;
#pragma link C++ typedef ROOT::Math::IMultiGradFunction;


#ifdef OLD

#pragma link C++ class ROOT::Math::IBaseFunction<ROOT::Math::MultiDim>+;
#pragma link C++ class ROOT::Math::IGradientFunction<ROOT::Math::MultiDim>+;

#pragma link C++ class ROOT::Math::IParametricFunction<ROOT::Math::OneDim>+;
#pragma link C++ class ROOT::Math::IParametricFunction<ROOT::Math::MultiDim>+;
#pragma link C++ class ROOT::Math::IParametricGradFunction<ROOT::Math::MultiDim>+;

// some problem in CINT with template specialized class 
#ifdef LATER
#pragma link C++ class ROOT::Math::IBaseFunction<ROOT::Math::OneDim>+;
#pragma link C++ class ROOT::Math::IGradientFunction<ROOT::Math::OneDim>+;
#pragma link C++ class ROOT::Math::IParametricGradFunction<ROOT::Math::OneDim>+;
#endif
#endif

#pragma link C++ class ROOT::Math::IBaseFunctionOneDim+;
#pragma link C++ class ROOT::Math::IGradientOneDim+;
#pragma link C++ class ROOT::Math::IGradientFunctionOneDim+;
#pragma link C++ class ROOT::Math::IBaseParam+;

#pragma link C++ class ROOT::Math::IParametricFunctionOneDim+;
#pragma link C++ class ROOT::Math::IParametricGradFunctionOneDim+;

#pragma link C++ class ROOT::Math::IBaseFunctionMultiDim+;
#pragma link C++ class ROOT::Math::IGradientMultiDim+;
#pragma link C++ class ROOT::Math::IGradientFunctionMultiDim+;
#pragma link C++ class ROOT::Math::IParametricFunctionMultiDim+;
#pragma link C++ class ROOT::Math::IParametricGradFunctionMultiDim+;


// #pragma link C++ class ROOT::Math::Functor<ROOT::Math::IBaseFunctionMultiDim>+;
// #pragma link C++ class ROOT::Math::Functor<ROOT::Math::IGradientFunctionMultiDim>+;
// #pragma link C++ class ROOT::Math::Functor1D<ROOT::Math::IBaseFunctionOneDim>+;
// #pragma link C++ class ROOT::Math::Functor1D<ROOT::Math::IGradientFunctionOneDim>+;



#pragma link C++ function ROOT::Math::erf( double );
#pragma link C++ function ROOT::Math::erfc( double );
#pragma link C++ function ROOT::Math::tgamma( double );
#pragma link C++ function ROOT::Math::lgamma( double );
#pragma link C++ function ROOT::Math::beta( double , double);

#pragma link C++ function ROOT::Math::binomial_pdf( unsigned int , double, unsigned int);
#pragma link C++ function ROOT::Math::breitwigner_pdf( double , double, double);
#pragma link C++ function ROOT::Math::cauchy_pdf( double , double, double);
#pragma link C++ function ROOT::Math::chisquared_pdf( double , double, double);
#pragma link C++ function ROOT::Math::exponential_pdf( double , double, double);
#pragma link C++ function ROOT::Math::fdistribution_pdf( double , double, double, double);
#pragma link C++ function ROOT::Math::gamma_pdf( double , double, double, double);
#pragma link C++ function ROOT::Math::gaussian_pdf( double , double, double);
#pragma link C++ function ROOT::Math::lognormal_pdf( double , double, double, double);
#pragma link C++ function ROOT::Math::normal_pdf( double , double, double);
#pragma link C++ function ROOT::Math::poisson_pdf( unsigned int , double);
#pragma link C++ function ROOT::Math::tdistribution_pdf( double , double, double);
#pragma link C++ function ROOT::Math::uniform_pdf( double , double, double, double);

#pragma link C++ function ROOT::Math::breitwigner_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::breitwigner_cdf( double , double, double);
#pragma link C++ function ROOT::Math::cauchy_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::cauchy_cdf( double , double, double);
#pragma link C++ function ROOT::Math::chisquared_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::chisquared_cdf( double , double, double);
#pragma link C++ function ROOT::Math::exponential_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::exponential_cdf( double , double, double);
#pragma link C++ function ROOT::Math::gaussian_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::gaussian_cdf( double , double, double);
#pragma link C++ function ROOT::Math::lognormal_cdf_c( double , double, double, double);
#pragma link C++ function ROOT::Math::lognormal_cdf( double , double, double, double);
#pragma link C++ function ROOT::Math::normal_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::normal_cdf( double , double, double);
#pragma link C++ function ROOT::Math::uniform_cdf_c( double , double, double, double);
#pragma link C++ function ROOT::Math::uniform_cdf( double , double, double, double);

#pragma link C++ function ROOT::Math::fdistribution_cdf_c( double , double, double, double);
#pragma link C++ function ROOT::Math::fdistribution_cdf( double , double, double, double);
#pragma link C++ function ROOT::Math::gamma_cdf_c( double , double, double, double);
#pragma link C++ function ROOT::Math::gamma_cdf( double , double, double, double);
#pragma link C++ function ROOT::Math::tdistribution_cdf_c( double , double, double);
#pragma link C++ function ROOT::Math::tdistribution_cdf( double , double, double);


#endif
