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
   void Init(UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar = 0, Double_t* params = NULL, Double_t* parMin = NULL, Double_t* parMax = NULL, Double_t* parErrors = NULL); // initalization function
   void StoreFunctor(const char *name);
   Double_t ConfigureAndMinimize(const ROOT::Math::IBaseFunctionMultiDim* func, Double_t* x = NULL, Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1e-10, Int_t maxIter = 1000) const;
   void UpdateParamPtrs(const Double_t* x, const Double_t* params = NULL) const;

   virtual Double_t DoDerivative(const Double_t* x, UInt_t icoord) const; // inherited ROOT::Math::IGradientMultiDim
   virtual Double_t DoEvalPar(const Double_t* x, const Double_t* params = NULL) const; // inherited ROOT::Math::IParametricFunctionMultiDim
   virtual Double_t DoParameterDerivative(const Double_t* x, const Double_t* p, UInt_t ipar) const; // inherited ROOT::Math::IParametricGradFunctionMultiDim
   
   UInt_t fNdim;          // number of dimensions (vector coordinates)
   Double_t* fMin;        //[fNdim] Lower bounds for the range
   Double_t* fMax;        //[fNdim] Upper bounds for the range

   UInt_t fNpar;          // number of parameters
   Double_t* fParams;     //[fNpar] Array of parameters
   Double_t* fParErrors;  //[fNpar] Array of errors of the fNpar parameters
   Double_t* fParMin;     //[fNpar] Array of lower limits of the fNpar parameters
   Double_t* fParMax;     //[fNpar] Array of upper limits of the fNpar parameters

   TObject     *fParent;  //!Parent object hooking this function (if one)

   enum EFunctor { FORMULA, FUNCTOR, INTERPRETER_FUNCTOR, EMPTY }; // type of construction parameter
   EFunctor fType;

   TFormula fFormula;    //!TFormula in case of standard function 
   ROOT::Math::ParamFunctor fFunctor;   //! Functor object to wrap any C++ callable object
   TMethodCall* fMethodCall; //!Pointer to MethodCall in case of interpreted function

   mutable ROOT::Math::DistSampler* fSampler;

public:
   TFn();
   TFn(const char* name, const char* formula, Double_t* min, Double_t* max);
   TFn(const char* name, UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar = 0);
#ifndef __CINT__
   // TODO: determine if there is still a reason to keep these constructors
//   TFn(const char* name, UInt_t ndim, Double_t (*fcn)(Double_t*, Double_t*), Double_t* min, Double_t* max, UInt_t npar = 0);
//   TFn(const char* name, UInt_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, UInt_t npar = 0);
#endif

   // Constructors using functors (compiled mode only)
   TFn(const char *name, UInt_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, UInt_t npar = 0);

   // Template constructors from any C++ callable object, defining "Double_t operator() (Double_t*, Double_t*)".
   // The class name is not needed when using compile code, while it is required when using
   // interpreted code via the specialized constructor with void *.
   // An instance of the C++ function class or its pointer can both be used. The former is recommended when using
   // C++ compiled code, but if CINT compatibility is needed, then a pointer to the function class must be used.
   // xmin and xmax specify the plotting range,  npar is the number of parameters.
   // See the tutorial math/exampleFunctor.C for an example of using this constructor
   template <typename Func>
   TFn(const char *name, UInt_t ndim, Func f, Double_t* min, Double_t* max, UInt_t npar = 0, const char* = NULL ) :
      TNamed(name, "TFn created by a templated constructor from any C++ functor"),
      fParent(NULL),
      fType(FUNCTOR),
      fFormula(),
      fFunctor(ROOT::Math::ParamFunctor(f)),
      fMethodCall(NULL)
   {
      StoreFunctor(name);
      Init(ndim, min, max, npar);
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
   TFn(const char *name, UInt_t ndim, const PtrObj& p, MemFn memFn, Double_t* min, Double_t* max, UInt_t npar = 0, const char* = NULL, const char* = NULL) :
      TNamed(name, "TFn created from a PtrObj"),
      fParent(NULL),
      fType(FUNCTOR),
      fFormula(),
      fFunctor(ROOT::Math::ParamFunctor(p, memFn)),
      fMethodCall(NULL)
   {
      StoreFunctor(name);
      Init(ndim, min, max, npar);
   }

   virtual ~TFn();
   TFn(const TFn &rhs, const char *name = NULL);
   TFn& operator=(const TFn &rhs);
   virtual TFn* Clone(const char* name) const { return new TFn(*this, name); } // inherited TObject
   virtual TFn* Clone() const { return new TFn(*this); } // inherited ROOT::Math::IBaseFunctionMultiDim

   virtual void      FixParameter(UInt_t ipar, Double_t value); // fixes parameter in a single point
   virtual void      FdF(const Double_t* x, Double_t& f, Double_t* df) const; // inherited ROOT::Math::IGradientMultiDim
   virtual THn*      GetHistogram(const UInt_t* npoints) const;
   virtual Double_t  GetMaximum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1e-10, Int_t maxIter = 1000) const;
   virtual Double_t  GetMinimum (Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1e-10, Int_t maxIter = 1000) const;
   virtual Double_t* GetMaximumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1e-10, Int_t maxIter = 1000) const;
   virtual Double_t* GetMinimumX(Double_t* min = NULL, Double_t* max = NULL, Double_t epsilon = 1e-10, Int_t maxIter = 1000) const;
   virtual UInt_t    GetNumberFreeParameters() const;
   virtual TObject*  GetParent() const { return fParent; }
   virtual Double_t  GetParError(UInt_t ipar) const;
   virtual Double_t* GetParErrors() const { return fParErrors; }
   virtual void      GetParLimits(UInt_t ipar, Double_t &parmin, Double_t &parmax) const;
   virtual const Double_t* GetRandom() const; // Return a sample random number following this distribution
   virtual void      GetRange(Double_t*& min, Double_t*& max) const;
   virtual void      Gradient(const Double_t* x, Double_t* grad) const; // inherited ROOT::Math::IGradientMultiDim
   virtual Double_t  IntegralError(const Double_t* a, const Double_t* b, const Double_t* params = NULL, const Double_t* covmat = NULL, Double_t eps = 1e-6);
   virtual Double_t  Integral(const Double_t* a = NULL, const Double_t* b = NULL, Double_t absTol = 1e-6, Double_t relTol = 1e-6, UInt_t maxEval = 100000, Double_t* error = NULL, Int_t* status = NULL) const;
   virtual Bool_t    IsInside(const Double_t *x) const;
   virtual UInt_t    NDim() const { return fNdim; } // Inherited from ROOT::Math::IBaseFunctionMultiDim
   virtual UInt_t    NPar() const { return fNpar; } // inherited ROOT::Math::IBaseParam
   virtual const Double_t* Parameters() const { return fParams; } // inherited ROOT::Math::IBaseParam
   virtual TF1*      Projection1D(UInt_t icoord) const;
   virtual void      Print(Option_t *option="") const;
   virtual void      ReleaseParameter(UInt_t ipar); // opposite to FixParameter
   virtual void      SetParameters(const Double_t* p); // inherited ROOT::Math::IBaseParam
   virtual void      SetParError(UInt_t ipar, Double_t error);
   virtual void      SetParErrors(const Double_t *errors);
   virtual void      SetParLimits(UInt_t ipar, Double_t parmin, Double_t parmax);
   virtual void      SetParent(TObject *p=0) { fParent = p;}
   virtual void      SetRange(Double_t* min, Double_t* max);

   ClassDef(TFn, 1)  //The Parametric n-D function
};


#endif // ROOT_TFn


