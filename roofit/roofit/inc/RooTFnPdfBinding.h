/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef ROOTFNPDFBINDING
#define ROOTFNPDFBINDING

#include "RooListProxy.h"
#include "RooAbsPdf.h"
class TF1 ;
class TF2 ;
class TF3 ;

class RooTFnPdfBinding : public RooAbsPdf {
public:
  RooTFnPdfBinding() {} ; 
  RooTFnPdfBinding(const char *name, const char *title, TF1* _func, const RooArgList& _list);
  RooTFnPdfBinding(const RooTFnPdfBinding& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new RooTFnPdfBinding(*this,newname); }
  inline virtual ~RooTFnPdfBinding() { }

  void printArgs(std::ostream& os) const ;

protected:

  RooListProxy list ;
  TF1* func ;
  
  Double_t evaluate() const ;

private:

  ClassDef(RooTFnPdfBinding,1) // RooAbsPdf binding to ROOT TF[123] functions
};
 

namespace RooFit {

RooAbsPdf* bindPdf(TF1* func,RooAbsReal& x) ;
RooAbsPdf* bindPdf(TF2* func,RooAbsReal& x, RooAbsReal& y) ;
RooAbsPdf* bindPdf(TF3* func,RooAbsReal& x, RooAbsReal& y, RooAbsReal& z) ;

}


#endif
