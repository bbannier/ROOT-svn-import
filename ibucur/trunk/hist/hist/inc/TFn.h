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
#ifndef ROOT_Math_IntegratorMultiDim
#include "Math/IntegratorMultiDim.h"
#endif
#ifndef ROOT_Math_DistSampler
#include "Math/DistSampler.h"
#endif

class TF1;
class TMethodCall;
class THn;

class TFn : public TNamed, public ROOT::Math::IParametricGradFunctionMultiDim, public ROOT::Math::IGradientMultiDim {

private:
   void Init(UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar = 0);
   virtual Double_t ConfigureAndMinimize(ROOT::Math::IBaseFunctionMultiDim* func, Double_t* x = NULL, Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 100) const;
   virtual Double_t DoParameterDerivative(const Double_t* x, const Double_t* p, UInt_t ipar) const {
      // TODO: implement
      return 0.0;
   }
   virtual Double_t DoEvalPar(const Double_t* x, const Double_t* params = NULL) const; // inherited ROOT::Math::IParametricFunctionMultiDim
   virtual Double_t DoDerivative(const Double_t* x, UInt_t icoord) const;
   
   UInt_t      fNdim;
   Double_t*   fMin;         //[fNdim ]Lower bounds for the range
   Double_t*   fMax;         //[fNdim] Upper bounds for the range

   UInt_t      fNpar;
   Double_t    *fParams;     //[fNpar] Array of parameters
   Double_t    *fParErrors;  //[fNpar] Array of errors of the fNpar parameters
   Double_t    *fParMin;     //[fNpar] Array of lower limits of the fNpar parameters
   Double_t    *fParMax;     //[fNpar] Array of upper limits of the fNpar parameters

   Double_t    fNorm;
   ROOT::Math::IntegratorMultiDim fIntegrator;
   ROOT::Math::DistSampler* fSampler;

   TObject     *fParent;     //!Parent object hooking this function (if one)
   // TODO: change to an internal enum
   Int_t        fType;        // 0 for standard functions, 
                             // 1 if pointer to function)

   TFormula    *fFormula;    //!Pointer to TFormula in case of standard function 
   TMethodCall *fMethodCall; //!Pointer to MethodCall in case of interpreted function
   void        *fCintFunc;              //! pointer to interpreted function class
   ROOT::Math::ParamFunctor fFunctor;   //! Functor object to wrap any C++ callable object

   void CreateFromFunctor(const char *name);
   void CreateFromCintClass(const char *name, Int_t ndim, void * ptr, Double_t* min, Double_t* max, Int_t npar, const char * cname, const char * fname);

public:
   TFn();
   TFn(const char* name, const char* formula, Double_t* min, Double_t* max);
   TFn(const char* name, Int_t ndim, void* fcn, Double_t* min, Double_t* max, Int_t npar = 0);
#ifndef __CINT__
   TFn(const char* name, UInt_t ndim, Double_t (*fcn)(Double_t*, Double_t*), Double_t* min, Double_t* max, UInt_t npar = 0);
   TFn(const char* name, Int_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, Int_t npar = 0);
#endif

   // Constructors using functors (compiled mode only)
   TFn(const char *name, Int_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, Int_t npar = 0);

   // Template constructors from any C++ callable object, defining "Double_t operator() (Double_t*, Double_t*)".
   // The class name is not needed when using compile code, while it is required when using
   // interpreted code via the specialized constructor with void *.
   // An instance of the C++ function class or its pointer can both be used. The former is recommended when using
   // C++ compiled code, but if CINT compatibility is needed, then a pointer to the function class must be used.
   // xmin and xmax specify the plotting range,  npar is the number of parameters.
   // See the tutorial math/exampleFunctor.C for an example of using this constructor
   template <typename Func>
   TFn(const char *name, Int_t ndim, Func f, Double_t* min, Double_t* max, Int_t npar, const char * = 0  ) :
      TNamed(name, "TFn created by a templated constructor from any C++ functor"),
      fType      ( 1 ),
      fIntegrator(),
      fParent    ( 0 ),
      fMethodCall ( 0),
      fCintFunc  ( 0 ),
      fFunctor( ROOT::Math::ParamFunctor(f) )
   {
      Init(ndim, min, max, npar);
      CreateFromFunctor(name);
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
      fType      ( 1 ),
      fIntegrator(),
      fParent    ( 0 ),
      fMethodCall( 0 ),
      fCintFunc  ( 0 ),
      fFunctor   ( ROOT::Math::ParamFunctor(p,memFn) )
   {
      Init(ndim, min, max, npar);
      CreateFromFunctor(name);
   }

   // constructor used by CINT
   TFn(const char *name, Int_t ndim, void *ptr, Double_t* min, Double_t* max, Int_t npar, const char *className );
   TFn(const char *name, Int_t ndim, void *ptr, void *, Double_t* min, Double_t* max, Int_t npar, const char *className, const char *methodName = 0);

   virtual   ~TFn();
   TFn(const TFn &f1);
   TFn& operator=(const TFn &rhs);
   virtual TFn* Clone() const { return new TFn(*this); }

   virtual Double_t Eval(Double_t* x);
   //virtual Double_t  operator()(Double_t* x); 
   virtual Double_t  operator()(const Double_t* x, const Double_t* params = NULL);  
   virtual void      FixParameter(UInt_t ipar, Double_t value);
   virtual THn*      GetHistogram() const;
   virtual Double_t  GetMaximum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 1000) const;
   virtual Double_t  GetMinimum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 1000) const;
   virtual Double_t* GetMaximumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 1000) const;
   virtual Double_t* GetMinimumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1.E-10, Int_t maxiter = 1000) const;
        TMethodCall* GetMethodCall() const { return fMethodCall; }
   virtual UInt_t    GetNumberFreeParameters() const;
           TObject*  GetParent() const { return fParent; }
   virtual Double_t  GetParError(UInt_t ipar) const;
   virtual Double_t* GetParErrors() const { return fParErrors; }
   virtual void      GetParLimits(UInt_t ipar, Double_t &parmin, Double_t &parmax) const;
   // Return a sample random number following this distribution
   // TODO: normalize
   virtual const Double_t* GetRandom() const { return fSampler->Sample(); }
   virtual void      GetRange(Double_t* min, Double_t* xmax) const;
   // GradientPar returns a vector of size n (dimensions), containing the gradient in the point specified by x
   virtual Double_t  GradientPar(UInt_t ipar, const Double_t* x, Double_t eps = 0.01) const;
   virtual void      GradientPar(const Double_t* x, Double_t* grad, Double_t eps = 0.01) const;
   virtual void      InitArgs(const Double_t* x, const Double_t *params = NULL);
   virtual Double_t  IntegralError(const Double_t* a, const Double_t* b, const Double_t* params = NULL, const Double_t* covmat = NULL, Double_t eps = 1e-6);
   virtual Double_t  IntegralMultiple(const Double_t* a, const Double_t* b, Int_t minPts, Int_t maxPts, Double_t eps, Double_t& relErr,Int_t& nFuncEval, Int_t& status);
   virtual Double_t  IntegralMultiple(const Double_t* a, const Double_t* b, Double_t eps, Double_t& relErr);
   virtual Bool_t    IsInside(const Double_t *x) const;
   virtual UInt_t    NDim() const { return fNdim; } // Inherited from ROOT::Math::IBaseFunctionMultiDim
   virtual Double_t  Norm() const { return fNorm; }
           TF1*      Projection1D(UInt_t idxCoord) const;
   virtual void      Print(Option_t *option="") const;
   virtual void      ReleaseParameter(UInt_t ipar);
   template <class PtrObj, typename MemFn> 
           void      SetFunction( PtrObj& p, MemFn memFn );
   template <typename Func> 
           void      SetFunction( Func f );
   virtual void      SetParError(UInt_t ipar, Double_t error);
   virtual void      SetParErrors(const Double_t *errors);
   virtual void      SetParLimits(UInt_t ipar, Double_t parmin, Double_t parmax);
   virtual void      SetParent(TObject *p=0) { fParent = p;}
   virtual void      SetRange(Double_t* min, Double_t* max);
   virtual const Double_t* Parameters() const { return fParams; } // inherited ROOT::Math::IBaseParam
   virtual void SetParameters(const Double_t* p) { // inherited ROOT::Math::IBaseParam
      if (!p) {
         Error("TFn::SetParameters", "Input parameter array not allocated");
         return;
      }
      std::copy(p, p + fNpar, fParams);
   }
   virtual UInt_t NPar() const { return fNpar; }
   virtual void FdF(const Double_t* x, Double_t& f, Double_t* df) const {}
   virtual void Gradient(const Double_t* x, Double_t* grad) const;

   ClassDef(TFn,1)  //The Parametric n-D function
};

inline Double_t TFn::operator()(const Double_t *x, const Double_t *params)
   { 
      if (fMethodCall) InitArgs(x,params);
      return DoEvalPar(x,params); 
   }

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

#endif // ROOT_TFn


