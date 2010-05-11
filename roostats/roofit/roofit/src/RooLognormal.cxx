 /***************************************************************************** 
  * Project: RooFit                                                           * 
  * @(#)root/roofit:$Id: RooLognormal.cxx 25185 2008-08-20 14:00:42Z wouter $ *
  *                                                                           * 
  * RooFit Lognormal PDF                                                      *
  *                                                                           * 
  * Author: Gregory Schott and Stefan Schmitz                                 *
  *                                                                           * 
  *****************************************************************************/ 

//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
// RooFit Lognormal PDF. The two parameters are:
//  - m0: the median of the distribution
//  - k=exp(sigma): sigma is called the shape parameter in the TMath parametrization
//
//   Lognormal(x,m0,k) = 1/(sqrt(2*pi)*ln(k)*x)*exp(-ln^2(x/m0)/(2*ln^2(k)))
//
// The parametrization here is physics driven and differs from the ROOT::Math::lognormal_pdf(x,m,s,x0) with:
//  - m = log(m0)
//  - s = log(k)
//  - x0 = 0
// END_HTML


#include "RooFit.h"

#include "Riostream.h"
#include "Riostream.h"
#include <math.h>

#include "RooLognormal.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooRandom.h"
#include "RooMath.h"

#include <Math/SpecFuncMathCore.h>
#include <Math/PdfFuncMathCore.h>
#include <Math/ProbFuncMathCore.h>

ClassImp(RooLognormal)


//_____________________________________________________________________________
RooLognormal::RooLognormal(const char *name, const char *title,
			 RooAbsReal& _x, RooAbsReal& _m0,
			 RooAbsReal& _k) :
  RooAbsPdf(name,title),
  x("x","Observable",this,_x),
  m0("m0","m0",this,_m0),
  k("k","k",this,_k)
{
}



//_____________________________________________________________________________
RooLognormal::RooLognormal(const RooLognormal& other, const char* name) : 
  RooAbsPdf(other,name), x("x",this,other.x), m0("m0",this,other.m0),
  k("k",this,other.k)
{
}



//_____________________________________________________________________________
Double_t RooLognormal::evaluate() const
{
  Double_t xv = x;
  Double_t ln_k = log(k);
  Double_t ln_m0 = log(m0);
  Double_t x0 = 0;

  Double_t ret = ROOT::Math::lognormal_pdf(xv,ln_m0,ln_k,x0);
  return ret ;
}



//_____________________________________________________________________________
Int_t RooLognormal::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const 
{
  //  if (matchArgs(allVars,analVars,x)) return 1 ;
  return 0 ;
}



//_____________________________________________________________________________
Double_t RooLognormal::analyticalIntegral(Int_t code, const char* rangeName) const 
{
  assert(code==1) ;

  /*
  static const Double_t root2 = sqrt(2.) ;
  static const Double_t rootPiBy2 = sqrt(atan2(0.0,-1.0)/2.0);
  
  Double_t xscale = root2*k;
  Double_t ret = rootPiBy2*k*(RooMath::erf((x.max(rangeName)-m0)/xscale)-RooMath::erf((x.min(rangeName)-m0)/xscale));

  //cout << "Int_gauss_dx(m0=" << m0 << ",k=" << k << ", xmin=" << x.min(rangeName) << ", xmax=" << x.max(rangeName) << ")=" << ret << endl ;
  return ret ;
  */
  return 1;
}




//_____________________________________________________________________________
Int_t RooLognormal::getGenerator(const RooArgSet& directVars, RooArgSet &generateVars, Bool_t /*staticInitOK*/) const
{
  //if (matchArgs(directVars,generateVars,x)) return 1 ;  
  return 0 ;
}



//_____________________________________________________________________________
void RooLognormal::generateEvent(Int_t code)
{
  assert(code==1) ;
  /*
  Double_t xgen ;
  while(1) {    
    xgen = RooRandom::randomGenerator()->Gaus(m0,k);
    if (xgen<x.max() && xgen>x.min()) {
      x = xgen ;
      break;
    }
  }
  */
  return;
}


