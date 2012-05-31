/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 * @(#)root/roofit:$Id$
 * Authors:                                                                  *
 *   Kyle Cranmer
 *                                                                           *
 *****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
// Bernstein basis polynomials are positive-definite in the range [0,1].
// In this implementation, we extend [0,1] to be the range of the parameter.
// There are n+1 Bernstein basis polynomials of degree n.
// Thus, by providing N coefficients that are positive-definite, there 
// is a natural way to have well bahaved polynomail PDFs.
// For any n, the n+1 basis polynomials 'form a partition of unity', eg.
//  they sum to one for all values of x. See
// http://www.idav.ucdavis.edu/education/CAGDNotes/Bernstein-Polynomials.pdf
// END_HTML
//

#include "RooFit.h"

#include "Riostream.h"
#include "Riostream.h"
#include <math.h>
#include "TMath.h"
#include "RooBernstein.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooArgList.h"

ClassImp(RooBernstein)
;


//_____________________________________________________________________________
RooBernstein::RooBernstein()
{
}


//_____________________________________________________________________________
RooBernstein::RooBernstein(const char* name, const char* title, 
                           RooAbsReal& x, const RooArgList& coefList): 
  RooAbsPdf(name, title),
  _x("x", "Dependent", this, x),
  _coefList("coefficients","List of coefficients",this)
{
  // Constructor
  degree = coefList.getSize() - 1; // n + 1 polys of degree n
  coefs = new RooAbsReal *[degree + 1];
  TIterator* coefIter = coefList.createIterator();

  for(Int_t i = 0; i <= degree; i++) {
    if ( (coefs[i] = dynamic_cast<RooAbsReal*>(coefIter->Next())) == NULL) {
      cout << "RooBernstein::ctor(" << GetName() << ") ERROR: coefficient " << coefs[i]->GetName() 
	   << " is not of type RooAbsReal" << endl ;
      assert(0) ;
    }
  }
  
  c = new Double_t[degree + 1];
  b = new Double_t[degree + 1];
  
  for(Int_t i = 0; i <= degree; i++) b[i] = TMath::Binomial(degree,i);

  delete coefIter ;
}



//_____________________________________________________________________________
RooBernstein::RooBernstein(const RooBernstein& other, const char* name) :
  RooAbsPdf(other, name), 
  _x("x", this, other._x), 
  _coefList("coefList",this,other._coefList)
{
}


//_____________________________________________________________________________
Double_t RooBernstein::evaluate() const 
{

  Double_t xmin = _x.min();  
  Double_t x = (_x - xmin) / (_x.max() - xmin);



/*  if(degree == 0) {
    return coefs[0]->getVal();

  } else if(degree == 1) {
    Double_t a0 = coefs[0]->getVal(); // c0
    Double_t a1 = coefs[1]->getVal() - a0; // c1 - c0 
    return a1 * x + a0;

  } else if(degree == 2) {
    Double_t a0 = coefs[0]->getVal(); // c0
    Double_t a1 = 2 * (coefs[1]->getVal() - a0); // 2 * (c1 - c0)
    Double_t a2 = coefs[2]->getVal() - a1 - a0; // c0 - 2 * c1 + c2
    return (a2 * x + a1) * tempx + a0;

  } else if(degree == 3) { 
    Double_t a0 = coefs[0]->getVal(); // c0
    Double_t a1 = 3 * (coefs[1]->getVal() - a0); // 3 * (c1 - c0)
    Double_t a2 = (coefs[2]->getVal() - a0) * 3 - a1 * 2;
    Double_t a3 = coefs[3]->getVal() - a2 - a1 - a0;
    return ((a3 * x + a2) * x + a1) * x + a0;

  } else {*/

    // MY ALGORITHM
    
/*    for(Int_t i = degree; i >= 0; i--) {
      c[i] = coefs[i]->getVal();
      for(Int_t j = i; j < degree; j++) {
        c[j + 1] -= c[j];
      }
    }


    Double_t temp = c[degree];

    for(Int_t i = degree - 1; i >= 1 ; i--) 
      temp = temp * x + c[i] * b[i];
    temp = temp * x + c[0];
*/
   
   Double_t t = x;
   Double_t s = 1 - x;
   Double_t temp = coefs[0]->getVal() * s;

   for(Int_t i = 1; i < degree; i++) {
      temp = (temp + t * b[i] * coefs[i]->getVal()) * s; 
      t *= x;
   }
   temp += t * coefs[degree]->getVal();

/*  }*/
   
/*    Double_t temp2 = 0;

    for (int i=0; i <= degree; ++i){
      temp2 += coefs[i]->getVal() *
        b[i] * pow(x,i) * pow(1-x,degree-i);
    }

    cout << setprecision(16) << "temp " << temp << " temp2 " << temp2 << endl;
    assert(temp == temp2);
*/
  return temp;
}


//_____________________________________________________________________________
Int_t RooBernstein::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName) const 
{
  // No analytical calculation available (yet) of integrals over subranges
  if (rangeName && strlen(rangeName)) {
    return 0 ;
  }

  if (matchArgs(allVars, analVars, _x)) return 1;
  return 0;
}


//_____________________________________________________________________________
Double_t RooBernstein::analyticalIntegral(Int_t code, const char* rangeName) const 
{
  assert(code==1) ;
  Double_t xmin = _x.min(rangeName); Double_t xmax = _x.max(rangeName);
  Double_t norm(0) ;

  RooFIter iter = _coefList.fwdIterator() ;
  Double_t temp=0;
  for (int i=0; i<=degree; ++i){
    // for each of the i Bernstein basis polynomials
    // represent it in the 'power basis' (the naive polynomial basis)
    // where the integral is straight forward.
    temp = 0;
    for (int j=i; j<=degree; ++j){ // power basis≈ß
      temp += pow(-1.,j-i) * b[j] * TMath::Binomial(j,i) / (j+1);
    }
    temp *= ((RooAbsReal*)iter.next())->getVal(); // include coeff
    norm += temp; // add this basis's contribution to total
  }

  norm *= xmax-xmin;
  return norm;
}
