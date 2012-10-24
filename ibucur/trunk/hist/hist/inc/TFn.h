// @(#)root/hist:$Id$
// Author: Rene Brun   18/08/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
// ---------------------------------- F1.h

#ifndef ROOT_TFn
#define ROOT_TFn



//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TFn                                                                  //
//                                                                      //
// The Parametric n-D function                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TFormula
#include "TFormula.h"
#endif

#ifndef ROOT_Math_ParamFunctor
#include "Math/ParamFunctor.h"
#endif
#ifndef ROOT_Math_IParamFunction
#include "Math/IParamFunction.h"
#endif

class TFn;
class TH1;
class TAxis;
class TMethodCall;

namespace ROOT { 
   namespace Fit { 
      class FitResult; 
   }
}

// TODO: check abstract methods that need implementing in new base classes
class TFn : public TNamed, public ROOT::Math::IParametricGradFunctionMultiDim, public ROOT::Math::IGradientMultiDim {

private:
   void Init(Int_t ndim, Double_t* xmin, Double_t* xmax);
   virtual Double_t ConfigureAndMinimize(ROOT::Math::IBaseFunctionMultiDim* func, Double_t* x = NULL, Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;

protected:
   Double_t*   fMin;         //Lower bounds for the range
   Double_t*   fMax;         //Upper bounds for the range
   Int_t       fNpx;         //Number of points used for the graphical representation
   // TODO: change to an internal enum
   Int_t       fType;        // 0 for standard functions, 
                             // 1 if pointer to function)
   Int_t       fNpfits;      //Number of points used in the fit
   Int_t       fNDF;         //Number of degrees of freedom in the fit
   Int_t       fNdim;
   Int_t       fNpar;
   Int_t       fNsave;       //Number of points used to fill array fSave
   Double_t    *fIntegral;   //![fNpx] Integral of function binned on fNpx bins
   Double_t    *fParErrors;  //[fNpar] Array of errors of the fNpar parameters
   Double_t    *fParMin;     //[fNpar] Array of lower limits of the fNpar parameters
   Double_t    *fParMax;     //[fNpar] Array of upper limits of the fNpar parameters
   Double_t    *fSave;       //[fNsave] Array of fNsave function values
   Double_t    *fAlpha;      //!Array alpha. for each bin in x the deconvolution r of fIntegral
   Double_t    *fBeta;       //!Array beta.  is approximated by x = alpha +beta*r *gamma*r**2
   Double_t    *fGamma;      //!Array gamma.
   Double_t    *fParams;     //[fNpar] Array of parameters
   TObject     *fParent;     //!Parent object hooking this function (if one)
   TH1         *fHistogram;  //!Pointer to histogram used for visualisation
   Double_t     fMaximum;    //Maximum value for plotting
   Double_t     fMinimum;    //Minimum value for plotting

   TFormula    *fFormula;    //!Pointer to TFormula in case of standard function 
   TMethodCall *fMethodCall; //!Pointer to MethodCall in case of interpreted function
   void        *fCintFunc;              //! pointer to interpreted function class
   ROOT::Math::ParamFunctor fFunctor;   //! Functor object to wrap any C++ callable object

   static Bool_t fgAbsValue;  //use absolute value of function when computing integral
   static Bool_t fgRejectPoint;  //True if point must be rejected in a fit

   void CreateFromFunctor(const char *name, Int_t npar);
   void CreateFromCintClass(const char *name, Int_t ndim, void * ptr, Double_t* min, Double_t* max, Int_t npar, const char * cname, const char * fname);

public:
    // TFn status bits
    enum {
       kNotDraw     = BIT(9)  // don't draw the function when in a TH1
    };

   TFn();
   TFn(const char* name, const char* formula, Double_t* min, Double_t* max);
   TFn(const char* name, Int_t ndim, void* fcn, Double_t* min, Double_t* max, Int_t npar = 0);
#ifndef __CINT__
   TFn(const char* name, Int_t ndim, Double_t (*fcn)(Double_t*, Double_t*), Double_t* min, Double_t* max, Int_t npar = 0);
   TFn(const char* name, Int_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, Int_t npar = 0);
#endif

   // Constructors using functors (compiled mode only)
   TFn(const char *name, Int_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, Int_t npar = 0);

   // Template constructors from any  C++ callable object,  defining  the operator() (double * , double *)
   // and returning a double.
   // The class name is not needed when using compile code, while it is required when using
   // interpreted code via the specialized constructor with void *.
   // An instance of the C++ function class or its pointer can both be used. The former is reccomended when using
   // C++ compiled code, but if CINT compatibility is needed, then a pointer to the function class must be used.
   // xmin and xmax specify the plotting range,  npar is the number of parameters.
   // See the tutorial math/exampleFunctor.C for an example of using this constructor
   template <typename Func>
   TFn(const char *name, Int_t ndim, Func f, Double_t* min, Double_t* max, Int_t npar, const char * = 0  ) :
      TNamed(name, "TFn created by a templated constructor from any C++ functor"),
      fNpx       ( 100 ),
      fType      ( 1 ),
      fNpfits    ( 0 ),
      fNDF       ( 0 ),
      fNsave     ( 0 ),
      fIntegral  ( 0 ),
      fParErrors ( 0 ),
      fParMin    ( 0 ),
      fParMax    ( 0 ),
      fSave      ( 0 ),
      fAlpha     ( 0 ),
      fBeta      ( 0 ),
      fGamma     ( 0 ),
      fParent    ( 0 ),
      fHistogram ( 0 ),
      fMaximum   ( -1111 ),
      fMinimum   ( -1111 ),
      fMethodCall ( 0),
      fCintFunc  ( 0 ),
      fFunctor( ROOT::Math::ParamFunctor(f) )
   {
      Init(ndim, min, max);
      CreateFromFunctor(name, npar);
   }

   // Template constructors from a pointer to any C++ class of type PtrObj with a specific member function of type
   // MemFn.
   // The member function must have the signature of  (double * , double *) and returning a double.
   // The class name and the method name are not needed when using compile code
   // (the member function pointer is used in this case), while they are required when using interpreted
   // code via the specialized constructor with void *.
   // xmin and xmax specify the plotting range,  npar is the number of parameters.
   // See the tutorial math/exampleFunctor.C for an example of using this constructor
   template <class PtrObj, typename MemFn>
   TFn(const char *name, Int_t ndim, const PtrObj& p, MemFn memFn, Double_t* min, Double_t* max, Int_t npar, const char * = 0, const char * = 0) :
      TNamed     (name, "TFn created from a PtrObj"),
      fNpx       ( 100 ),
      fType      ( 1 ),
      fNpfits    ( 0 ),
      fNDF       ( 0 ),
      fNsave     ( 0 ),
      fIntegral  ( 0 ),
      fParErrors ( 0 ),
      fParMin    ( 0 ),
      fParMax    ( 0 ),
      fSave      ( 0 ),
      fAlpha     ( 0 ),
      fBeta      ( 0 ),
      fGamma     ( 0 ),
      fParent    ( 0 ),
      fHistogram ( 0 ),
      fMaximum   ( -1111 ),
      fMinimum   ( -1111 ),
      fMethodCall( 0 ),
      fCintFunc  ( 0 ),
      fFunctor   ( ROOT::Math::ParamFunctor(p,memFn) )
   {
      Init(ndim, min, max);
      CreateFromFunctor(name, npar);
   }

   // constructor used by CINT
   TFn(const char *name, Int_t ndim, void *ptr, Double_t* min, Double_t* max, Int_t npar, const char *className );
   TFn(const char *name, Int_t ndim, void *ptr, void *, Double_t* min, Double_t* max, Int_t npar, const char *className, const char *methodName = 0);

   TFn(const TFn &f1);
   TFn& operator=(const TFn &rhs);
   virtual   ~TFn();
   virtual void     Copy(TObject &f1) const;
   // TODO: see versus parametrised
   virtual Double_t DoEval(Double_t *) const {
      //return EvalPar(x, fParams); //TODO: check how parameters can be modified
      // TODO: see how to return absolute value
      // TODO: solve constness issue - Why is EvalPar not constant?
      return 0.0;
   }
   virtual Double_t Eval(Double_t* x);
   virtual Double_t EvalPar(const Double_t *x, const Double_t *params =  NULL);
   // for using TFn as a callable object (functor)
   virtual Double_t operator()(Double_t* x); 
   virtual Double_t operator()(const Double_t* x, const Double_t* params = NULL);  
   virtual void     FixParameter(Int_t ipar, Double_t value);
   //        TH1     *GetHistogram() const; // XXX: THn?
   virtual Double_t  GetMaximum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;
   virtual Double_t  GetMinimum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;
   virtual Double_t* GetMaximumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;
   virtual Double_t* GetMinimumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;
   virtual Int_t    GetNDF() const;
   virtual Int_t    GetNpx() const {return fNpx;}
    TMethodCall    *GetMethodCall() const {return fMethodCall;}
   virtual Int_t    GetNumberFreeParameters() const;
   virtual Int_t    GetNumberFitPoints() const {return fNpfits;}
        TObject    *GetParent() const {return fParent;}
   virtual Double_t GetParError(Int_t ipar) const;
   virtual Double_t *GetParErrors() const {return fParErrors;}
   virtual void     GetParLimits(Int_t ipar, Double_t &parmin, Double_t &parmax) const;
   virtual Double_t GetRandom();
   virtual void     GetRange(Double_t* min, Double_t* xmax) const;
   virtual Double_t GetSave(const Double_t *x);
   // TODO: GetAxis (maybe)
   // TODO: GetPoint instead of GetX -> should be possible
//virtual Double_t GetX(Double_t y, Double_t xmin=0, Double_t xmax=0, Double_t epsilon = 1.E-10, Int_t maxiter = 100, Bool_t logx = false) const;
//   virtual Double_t GetXmin() const {return fXmin;}
//   virtual Double_t GetXmax() const {return fXmax;}.
   virtual Double_t GradientPar(Int_t ipar, const Double_t *x, Double_t eps=0.01);
   virtual void     GradientPar(const Double_t *x, Double_t *grad, Double_t eps=0.01);
   virtual void     InitArgs(const Double_t *x, const Double_t *params = NULL);
   virtual Double_t IntegralError(Double_t a, Double_t b, const Double_t *params=0, const Double_t *covmat=0, Double_t epsilon=1e-12);
   virtual Double_t IntegralError(Int_t n, const Double_t * a, const Double_t * b, const Double_t *params=0, const Double_t *covmat=0, Double_t epsilon=1e-12);
   //virtual Double_t IntegralFast(const TGraph *g, Double_t a, Double_t b, Double_t *params=0);
   virtual Double_t IntegralMultiple(Int_t n, const Double_t *a, const Double_t *b, Int_t minpts, Int_t maxpts, Double_t epsilon, Double_t &relerr,Int_t &nfnevl, Int_t &ifail);
   virtual Double_t IntegralMultiple(Int_t n, const Double_t *a, const Double_t *b, Double_t epsilon, Double_t &relerr);
   virtual Bool_t   IsInside(const Double_t *x) const;
   virtual void     Print(Option_t *option="") const;
   virtual void     ReleaseParameter(Int_t ipar);
   virtual void     Save(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax, Double_t zmin, Double_t zmax);
   virtual void     SavePrimitive(std::ostream &out, Option_t *option = "");
   virtual void     SetFitResult(const ROOT::Fit::FitResult & result, const Int_t * indpar = 0);
   template <class PtrObj, typename MemFn> 
   void SetFunction( PtrObj& p, MemFn memFn );
   template <typename Func> 
   void SetFunction( Func f );
   virtual void     SetNDF(Int_t ndf);
   virtual void     SetNumberFitPoints(Int_t npfits) {fNpfits = npfits;}
   virtual void     SetNpx(Int_t npx=100); // *MENU*
   virtual void     SetParError(Int_t ipar, Double_t error);
   virtual void     SetParErrors(const Double_t *errors);
   virtual void     SetParLimits(Int_t ipar, Double_t parmin, Double_t parmax);
   virtual void     SetParent(TObject *p=0) {fParent = p;}
   virtual void     SetRange(Double_t* min, Double_t* max); // *MENU*
   virtual void     SetSavedPoint(Int_t point, Double_t value);
   virtual void     Update();

   static  void     AbsValue(Bool_t reject=kTRUE);
   static  void     RejectPoint(Bool_t reject=kTRUE);
   static  Bool_t   RejectedPoint();

   // TODO: write a multi-dim version
   //Moments
   virtual Double_t Moment(Double_t n, Double_t* a, Double_t* b, const Double_t *params=0, Double_t epsilon=0.000001);
   virtual Double_t CentralMoment(Double_t n, Double_t* a, Double_t* b, const Double_t *params=0, Double_t epsilon=0.000001);
   virtual Double_t Mean(Double_t* a, Double_t* b, const Double_t *params=0, Double_t epsilon=0.000001) {return Moment(1,a,b,params,epsilon);}
   virtual Double_t Variance(Double_t* a, Double_t* b, const Double_t *params=0, Double_t epsilon=0.000001) {return CentralMoment(2,a,b,params,epsilon);}

   ClassDef(TFn,1)  //The Parametric n-D function
};

// XXX: maybe do not need both, or Eval method
inline Double_t TFn::operator()(Double_t *x) { return Eval(x); }
inline Double_t TFn::operator()(const Double_t *x, const Double_t *params)
   { 
      if (fMethodCall) InitArgs(x,params);
      return EvalPar(x,params); 
   }

/*
inline void TFn::SetRange(Double_t xmin, Double_t,  Double_t xmax, Double_t)
   { TFn::SetRange(xmin, xmax); }
inline void TFn::SetRange(Double_t xmin, Double_t, Double_t,  Double_t xmax, Double_t, Double_t)
   { TFn::SetRange(xmin, xmax); }
*/

template <typename Func> 
void TFn::SetFunction( Func f )    {
   // set function from a generic C++ callable object 
   fType = 1; 
   fFunctor = ROOT::Math::ParamFunctor(f); 
} 
template <class PtrObj, typename MemFn> 
void TFn::SetFunction( PtrObj& p, MemFn memFn )   { 
   // set from a pointer to a member function
   fType = 1; 
   fFunctor = ROOT::Math::ParamFunctor(p,memFn); 
} 

#endif
