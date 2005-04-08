// $Id: TFoamMaxwt.h,v 1.2 2005/04/04 10:59:34 psawicki Exp $

#ifndef ROOT_TFoamMaxwt
#define ROOT_TFoamMaxwt

//////////////////////////////////////////////////////////////////
//                                                              //
// Small auxiliary class for controlling MC weight.             //
//                                                              //
//////////////////////////////////////////////////////////////////

#include "TH1.h"

class TFoamMaxwt : public TObject {
 private:
  Double_t  fNent;      // No. of MC events
  Int_t     fnBin;      // No. of bins on the weight distribution
  Double_t  fwmax;      // Maximum analysed weight
 public:
  TH1D   *fWtHst1;      // Histogram of the weight wt
  TH1D   *fWtHst2;      // Histogram of wt filled with wt
 public:
  TFoamMaxwt();                            // NOT IMPLEMENTED (NEVER USED)
  TFoamMaxwt(const Double_t, const Int_t); // Principal Constructor
  TFoamMaxwt(TFoamMaxwt &From);            // Copy constructor
  virtual ~TFoamMaxwt();                   // Destructor
  void Reset();                            // Reset
  TFoamMaxwt& operator =(TFoamMaxwt &);    // operator =
  void Fill(Double_t);
  void Make(const Double_t, Double_t&);
  void GetMCeff(const Double_t, Double_t&, Double_t&);  // get MC efficiency= <w>/wmax
////////////////////////////////////////////////////////////////////////////
  ClassDef(TFoamMaxwt,1); //Controling of the MC weight (maximum weight)
};
#endif
