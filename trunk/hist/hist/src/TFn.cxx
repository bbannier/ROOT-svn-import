// @(#)root/hist:$Id$
// Author: Rene Brun   18/08/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <limits>

#include "Riostream.h"
#include "TROOT.h"
#include "TMath.h"
#include "TF1.h"
#include "TFn.h"
#include "THn.h"
#include "TRandom.h"
#include "TClass.h"
#include "TInterpreter.h"
#include "TPluginManager.h"
#include "TMethodCall.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "Math/IntegratorMultiDim.h"
#include "Math/BrentRootFinder.h"
#include "Math/BrentMethods.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/GaussIntegrator.h"
#include "Math/GaussLegendreIntegrator.h"
#include "Math/Integrator.h"
#include "Math/Minimizer.h"
#include "Math/RichardsonDerivator.h"
#include "Math/DistSampler.h"
#include "Fit/FitResult.h"


ClassImp(TFn)

//______________________________________________________________________________
/* 
Begin_Html
   <center><h2>TFn: n-Dim function class</h2></center>
   A TFn object is a n-Dim function defined between a lower and upper limit.
   <br>The function may be a simple function (see <tt>TFormula</tt>) or a
   precompiled user function.
   <br>The function may have associated parameters.
   <p>
      The following types of functions can be created:
      <ul>
         <li><a href="#F1">A - Expression using variable x and no parameters</a></li>
         <li><a href="#F2">B - Expression using variable x with parameters</a></li>
         <li><a href="#F3">C - A general C function with parameters</a></li>
         <li><a href="#F4">D - A general C++ function object (functor) with parameters</a></li>
         <li><a href="#F5">E - A member function with parameters of a general C++ class</a></li>
      </ul>

      <a name="F1"></a><h3>A - Expression using variable x and no parameters</h3>
      <h4>Case 1: inline expression using standard C++ functions/operators</h4>
      <div class="code"><pre>
         Double_t min = { ... }
         Double_t max = { ... }
      TFn *fn1 = new TFn("fn1","x[0] + sin(x[1])/x[1] + exp(x[2])", &min, &max);
      </pre></div><div class="clear" />
   </p>
End_Html
Begin_Macro
{
   TCanvas *c = new TCanvas("c","c",0,0,500,300);
   TFn *fa1 = new TFn("fa1","sin(x)/x",0,10);
   fa1->Draw();
   return c;
}
End_Macro
Begin_Html
   <h4>Case 2: inline expression using TMath functions without parameters</h4>
   <div class="code"><pre>
      TFn *fa2 = new TFn("fa2","TMath::DiLog(x)",0,10);
      fa2->Draw();
   </pre></div><div class="clear" />
End_Html
Begin_Macro
{
   TCanvas *c = new TCanvas("c","c",0,0,500,300);
   TFn *fa2 = new TFn("fa2","TMath::DiLog(x)",0,10);
   fa2->Draw();
   return c;
}
End_Macro
Begin_Html
   <h4>Case 3: inline expression using a CINT function by name</h4>
   <div class="code"><pre>
      Double_t myFunc(x) {
         return x+sin(x);
      }
      TFn *fa3 = new TFn("fa3","myFunc(x)",-3,5);
      fa3->Draw();
   </pre></div><div class="clear" />

   <a name="F2"></a><h3>B - Expression using variable x with parameters</h3>
   <h4>Case 1: inline expression using standard C++ functions/operators</h4>
   <ul>
      <li>Example a:
         <div class="code"><pre>
            TFn *fa = new TFn("fa","[0]*x*sin([1]*x)",-3,3);
         </pre></div><div class="clear" />
         This creates a function of variable x with 2 parameters.
         The parameters must be initialized via:
         <pre>
            fa->SetParameter(0,value_first_parameter);
            fa->SetParameter(1,value_second_parameter);
         </pre>
      </li>
      <li> Example b:
         <div class="code"><pre>
            TFn *fb = new TFn("fb","gaus(0)*expo(3)",0,10);
         </pre></div><div class="clear" />
         <tt>gaus(0)</tt> is a substitute for <tt>[0]*exp(-0.5*((x-[1])/[2])**2)</tt>
         and <tt>(0)</tt> means start numbering parameters at <tt>0</tt>.
         <tt>expo(3)</tt> is a substitute for <tt>exp([3]+[4]*x)</tt>.
      </li>
   </ul>

   <h4>Case 2: inline expression using TMath functions with parameters</h4>
   <div class="code"><pre>
      TFn *fb2 = new TFn("fa3","TMath::Landau(x,[0],[1],0)",-5,10);
      fb2->SetParameters(0.2,1.3);
      fb2->Draw();
   </pre></div><div class="clear" />
End_Html
Begin_Macro
{
   TCanvas *c = new TCanvas("c","c",0,0,500,300);
   TFn *fb2 = new TFn("fa3","TMath::Landau(x,[0],[1],0)",-5,10);
   fb2->SetParameters(0.2,1.3);
   fb2->Draw();
   return c;
}
End_Macro
Begin_Html
   <a name="F3"></a><h3>C - A general C function with parameters</h3>
   Consider the macro below:
   <div class="code"><pre>
      
      Double_t myfunction(Double_t *x, Double_t *par)
      {
         Float_t xx = x[0] * x[0] + x[1] * x[1];
         return TMath::Abs(par[0] * TMath::Sin(par[1] * xx) / xx);
      }
      void myfunc()
      {
         Double_t min[2] = {0.0, 0.0};
         Double_t max[2] = {10.0, 10.0};
         Double_t params[2] = {2.0, 1.0};
         TFn* fn = new TFn("myfunc", 2, myfunction, min, max, 2);
         f1->SetParameters(params);
      }
      void myhisto()
      { 
         TFn* fn = gROOT->GetFunction("myfunc");
         THn* hn = fn->GetHistogram();
      }
   </pre></div><div class="clear" />
End_Html
Begin_Html
   <p>
      In an interactive session you can do:
      <div class="code"><pre>
         Root > .L myfunc.C
         Root > myfunc();
         Root > myfit();
      </pre></div>
      <div class="clear" />
   </p>
End_Html
Begin_Html
   <tt>TFn</tt> objects can reference other <tt>TFn</tt> objects (thanks John
   Odonnell) of type A or B defined above. This excludes CINT interpreted functions
   and compiled functions. However, there is a restriction. A function cannot
   reference a basic function if the basic function is a polynomial polN.
   <p>Example:
      <div class="code"><pre>
         {
            Double_t min = {0.0, 0.0};
            Double_t max = {1.0, 1.0};

            TFn *fcos = new TFn ("fcos", "[0]*cos(x[0]) + cos(x[1])", min, max);
            fcos->SetParameter(0, 1.1);

            TFn *fsin = new TFn ("fsin", "[0]*sin(x[1]) - sin(x[0])", min, max);
            fsin->SetParameter(0, 2.1);

            TFn *fsincos = new TFn ("fsc", "fcos+fsin", min, max);
            TFn *fsum = new TFn ("fsum", "fsc+fsc", min, max);
         }
      </pre></div><div class="clear" />
   </p>
End_Html
Begin_Html
   <a name="F4"></a><h3>D - A general C++ function object (functor) with parameters</h3>
   A TFn can be created from any C++ class implementing the operator()(Double_t *x, Double_t *p).
   The advantage of the function object is that he can have a state and reference therefore what-ever other object.
   In this way the user can customize his function.
   <p>Example:
      <div class="code"><pre>
      class  MyFunctionObject {
       public:
         // use constructor to customize your function object

         Double_t operator() (Double_t *x, Double_t *p) {
            // function implementation using class data members
         }
      };
      {
          ....
         MyFunctionObject * fobj = new MyFunctionObject(....);       // create the function object
         TFn * f = new TFn("f",fobj,0,1,npar,"MyFunctionObject");    // create TFn class.
         .....
      }
      </pre></div><div class="clear" />
   <p>
   When constructing the TFn class, the name of the function object class is required only if running in CINT
   and it is not needed in compiled C++ mode. In addition in compiled mode the cfnution object can be passed to TFn
   by value.
   See also the tutorial math/exampleFunctor.C for a running example.
End_Html
Begin_Html
   <a name="F5"></a><h3>E - A member function with parameters of a general C++ class</h3>
   A TFn can be created in this case from any member function of a class which has the signature of
   (Double_t * , Double_t *) and returning a Double_t.
   <p>Example:
      <div class="code"><pre>
      class  MyFunction { // n-dimensional function
         public:
         ...
         Double_t Evaluate() (Double_t *x, Double_t *p) {
            // function implementation
         }
      };
      {
         ...
         MyFunction * fptr = new MyFunction(....);  // create the user function class
         TFn* f = new TFn("f", ndim, fptr, &MyFunction::Evaluate, 0,1,npar, "MyFunction", "Evaluate");   // create TFn class.
         ...
      }
      </pre></div><div class="clear" />
   </p>
   When constructing the TFn class, the name of the function class and of the member function are required only
   if running in CINT and they are not need in compiled C++ mode.
   See also the tutorial math/exampleFunctor.C for a running example.
End_Html */
class TFn_AbsValue : public ROOT::Math::IBaseFunctionMultiDim {
public:
   TFn_AbsValue(const TFn& func) : fFunc(func) {}
   virtual UInt_t NDim() const { return fFunc.NDim(); }
private:
   virtual TFn_AbsValue* Clone() const { return NULL; } // do not use
   const TFn& fFunc;
   virtual Double_t DoEval(const Double_t* x) const {
      Double_t result = fFunc(x);
      return result < 0 ? -result : result;
   }
};

/**
 * @class TFn_Distribution 
 * @brief Auxiliary class used in TFn::Init to initialize fSampler.
 *
 * Given a TFn representing a function f(x|p), where x is the input variable vector, p is the parameter 
 * vector, and p_i is the ith parameter, the user can create a new object equivalent to f normalized
 * over the range of x, by wrapping the TFn in a TFn_Distribution.
 *
 */
class TFn_Distribution : public ROOT::Math::IBaseFunctionMultiDim {
public:
   TFn_Distribution(const TFn& func) : fFunc(func), fIntegrator(fFunc) { 
      func.GetRange(fMin, fMax);
      fNormalisationConstant = fIntegrator.Integral(fMin, fMax);
   }
   ~TFn_Distribution() { delete[] fMin; delete[] fMax; }
   virtual UInt_t NDim() const { return fFunc.NDim(); }
private:
   virtual TFn_Distribution* Clone() const { return NULL; } // do not use
   const TFn_AbsValue fFunc;
   Double_t fNormalisationConstant;
   Double_t* fMin;
   Double_t* fMax;
   virtual Double_t DoEval(const Double_t* x) const { 
      if (fNormalisationConstant == 0.0) return 0.0;
      //std::cout << "TFn_Distribution::fNormalisationConstant " << fNormalisationConstant << std::endl;
      return fFunc(x) / fNormalisationConstant;
   }
   ROOT::Math::IntegratorMultiDim fIntegrator;
};

//______________________________________________________________________________
void TFn::Init(UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar, Double_t* params, Double_t* parMin, Double_t* parMax, Double_t* parErrors) 
{
   // TFn initializer, employed by constructors
   fMin = fMax = NULL;
   fParams = fParErrors = fParMin = fParMax = NULL;
   fNdim = fNpar = 0;
   fSampler = NULL;

   if (ndim > 0) {
      fNdim = ndim;
      fMin = new Double_t[fNdim];
      fMax = new Double_t[fNdim];
      std::copy(min, min + fNdim, fMin);
      std::copy(max, max + fNdim, fMax);   
   }   
   
   if (npar > 0) {
      fNpar = npar;
      fParams     = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];

      if (params) std::copy(params, params + fNpar, fParams);
      else std::fill(fParams, fParams + fNpar, 0);
      if (parMin) std::copy(parMin, parMin + fNpar, fParMin);
      else std::fill(fParMin, fParMin + fNpar, 0);
      if (parMax) std::copy(parMax, parMax + fNpar, fParMax);
      else std::fill(fParMax, fParMax + fNpar, 0);
      if (parErrors) std::copy(parErrors, parErrors + fNpar, fParErrors);
      else std::fill(fParErrors, fParErrors + fNpar, 0);
   } 
} 

//______________________________________________________________________________
TFn::TFn() : 
   TNamed(), 
   fParent(NULL), 
   fType(EMPTY), 
   fFormula(), 
   fFunctor(), 
   fMethodCall(NULL)
{
   // TFn default constructor.
   Init(0, NULL, NULL, 0);
}

//______________________________________________________________________________
TFn::TFn(const char* name, const char* formula, Double_t* min, Double_t* max) :
   TNamed(name, "TFn created from a formula definition (through TFormula)"),
   fParent(NULL),
   fType(FORMULA),
   fFormula(name, formula),
   fFunctor(),
   fMethodCall(NULL)
{
   // TFn constructor using a formula definition.
   // See TFormula for the explanation of its constructor syntax.
   // See tutorials: fillrandom, first, fit1, formula1, multifit for real examples.
   Init(fFormula.GetNdim(), min, max, fFormula.GetNpar());
}

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar):
   TNamed(name, "TFn created from the name of an interpreted function"),
   fParent(NULL),
   fType(INTERPRETER_FUNCTOR),
   fFormula(),
   fFunctor(),
   fMethodCall(NULL)
{
   /**
    * @brief TFn constructor using the name of an interpreted function.
    *
    * An object of type C in n-dimensional range [min, max] is created.
    * This constructor is caleld for functions of type C by CINT.
    * 
    * @param[in] min = lower bounds of vector coordinates
    * #param[in] max = upper bounds of vector coordinates 
    * @param[in] name = name of an interpreted CINT function
    * @param[in] npar = number of parameters
    *
    * @warning Function must be of type Double_t fcn(Double_t* x, Double_t* params)
    */
   Init(ndim, min, max, npar);

   TFn *fnOld = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(fnOld);
   
   if (name) {
      if(*name == '*') return; // is this possible?
      fMethodCall = new TMethodCall();
      fMethodCall->InitWithPrototype(name, "Double_t*,Double_t*");
      gROOT->GetListOfFunctions()->Add(this);
      if (!fMethodCall->IsValid())
         Error("TFn", "No function found with the signature Double_t %s(Double_t*, Double_t*)", name);
   } else {
      Error("TFn", "Requires a proper input function name!");
   }
}

/* XXX See if it can be removed
//______________________________________________________________________________
TFn::TFn(const char *name, UInt_t ndim, Double_t (*fcn)(Double_t *, Double_t *), Double_t* min, Double_t* max, UInt_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fParent(NULL),
   fType(FUNCTOR),
   fFormula(),
   fFunctor(fcn),
   fMethodCall(NULL)
{
   // TFn constructor using a pointer to a real function.
   //
   //   npar is the number of free parameters used by the function
   //
   //   This constructor creates a function of type C when invoked
   //   with the normal C++ compiler.
   //
   //   see test program test/stress.cxx (function stress1) for an example.
   //   note the interface with an intermediate pointer.
   //
   // WARNING! A function created with this constructor cannot be Cloned.
   Init(ndim, min, max, npar);
   StoreFunctor(name);
}

//______________________________________________________________________________
TFn::TFn(const char *name, UInt_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, UInt_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fParent(NULL),
   fType(FUNCTOR),
   fFormula(),
   fFunctor(ROOT::Math::ParamFunctor(fcn)),
   fMethodCall(NULL)
{
   // F1 constructor using a pointer to real function.
   //
   //   npar is the number of free parameters used by the function
   //
   //   This constructor creates a function of type C when invoked
   //   with the normal C++ compiler.
   //
   //   see test program test/stress.cxx (function stress1) for an example.
   //   note the interface with an intermediate pointer.
   //
   // WARNING! A function created with this constructor cannot be Cloned.
   Init(ndim, min, max, npar);
//   StoreFunctor(name);
}
*/

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, UInt_t npar ) :
   TNamed(name, "TFn created from ROOT::Math::ParamFunctor"),
   fParent(NULL),
   fType(FUNCTOR),
   fFormula(),
   fFunctor(f),
   fMethodCall(NULL)
{
   /** 
    * TFn constructor using the Functor class.
    *
    * @param [in] min and xmax define the range of the function
    * @param [in] npar is the number of free parameters used by the function
    *
    * WARNING! This constructor can be used only in compiled code
    * WARNING! A function created with this constructor cannot be Cloned.
    */
   StoreFunctor(name);
   Init(ndim, min, max, npar);
}


//______________________________________________________________________________
void TFn::StoreFunctor(const char* name)
{
   // Internal Function to Create a TFn  using a Functor.
   // Store formula in linked list of formula in ROOT
   TFn *fnOld = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   if(fnOld) gROOT->GetListOfFunctions()->Remove(fnOld);
   gROOT->GetListOfFunctions()->Add(this);
}


//______________________________________________________________________________
TFn::~TFn()
{
   // TFn destructor.
   //delete fSampler;
   delete [] fMin;
   delete [] fMax;
   delete [] fParams;
   delete [] fParMin;
   delete [] fParMax;
   delete [] fParErrors;
   
   if (gROOT) gROOT->GetListOfFunctions()->Remove(this);
   if (fParent) fParent->RecursiveRemove(this); fParent = NULL; 
}


//______________________________________________________________________________
TFn::TFn(const TFn &rhs, const char* name) : 
   TNamed(rhs),
   fParent(rhs.fParent),
   fType(rhs.fType),
   fFormula(rhs.fFormula),
   fFunctor(rhs.fFunctor)
{
   // TFn copy constructor.
   if (name) SetName(name);
   else SetName(TString::Format("%s_copy", rhs.GetName()));
   if (rhs.fMethodCall) fMethodCall = new TMethodCall(*rhs.fMethodCall);
   else fMethodCall = NULL;
   Init(rhs.fNdim, rhs.fMin, rhs.fMax, rhs.fNpar);
}

//______________________________________________________________________________
TFn& TFn::operator=(const TFn& rhs)
{
   // Assignment operator.

   if (this != &rhs) {
      fParent = rhs.fParent; 
      fType = rhs.fType;
      fFunctor = rhs.fFunctor;
      fFormula = rhs.fFormula;
      *fMethodCall = *rhs.fMethodCall;

      delete [] fMin; delete [] fMax;
      delete [] fParams; delete [] fParMin; delete [] fParMax; delete [] fParErrors;
      delete fSampler; 

      Init(rhs.fNdim, rhs.fMin, rhs.fMax, rhs.fNpar);
  }

   return *this;
}


class TFn_OneVariableCoordinate : public ROOT::Math::IBaseFunctionOneDim {
public:
   TFn_OneVariableCoordinate(const TFn& func, const Double_t* x, UInt_t icoord) : fFunc(func), fIdxCoord(icoord) {
      fX = new Double_t[func.NDim()];
      std::copy(x, x + func.NDim(), fX);
   }
   ~TFn_OneVariableCoordinate() { delete [] fX; }
private: 
   virtual ROOT::Math::IBaseFunctionOneDim* Clone() const { return NULL; }; // do not use

   const TFn& fFunc;
   const UInt_t fIdxCoord;
   Double_t* fX;
   virtual Double_t DoEval(Double_t x) const {
      fX[fIdxCoord] = x;
      return fFunc(fX);
   }
};

//______________________________________________________________________________
Double_t TFn::DoDerivative(const Double_t* x, UInt_t icoord) const
{
   /**   Calculate partial derivative of the function in point x, with respect to the 'icoord' coordonate
    *    The numerical method employs the central difference to compute the partial derivative. The precision
    *  of the computation is then improved via the Richardson Extrapolation.
    *    (http://math.fullerton.edu/mathews/n2003/richardsonextrapmod.html) 
    *    (http://cran.r-project.org/web/packages/numDeriv/index.html) 'R numDeriv package' 
    *  @param[in]      x - The point where the derivative is computed
    *  @param[in] icoord - The coordonate on which the derivation is performed. Must belong in [0, fNdim - 1].
    *  @return             Partial derivative (w.r.t. icoord) value in point x.
    */

   if (icoord > fNdim) {
      Error("DoDerivative", "The index of the %d-dimensional function %s is out of range", fNdim, GetName());
      return 0.0;
   }
   if (x == NULL) {
      Error("DoDerivative", "Vector of coordinates, x, passed as input is not allocated");
      return 0.0;
   }

   TFn_OneVariableCoordinate func(*this, x, icoord);
   ROOT::Math::RichardsonDerivator derivator(func);

   return derivator.Derivative1(x[icoord]);
}


//______________________________________________________________________________
Double_t TFn::DoEvalPar(const Double_t *x, const Double_t *params) const
{
   /**
    * @brief Evaluate function with given coordinates and parameters.
    *
    * Compute the value of this function at point defined by array x and current 
    * values of parameters in array params. If argument params is omitted, the 
    * internal values of parameters will be used instead.
   
    * @warning x must be filled with the corresponding number of dimensions.
    */
   if (params == NULL && fNpar > 0) {
      Info("DoEvalPar", "Parameter array not specified. Using internal parameters.");
      params = fParams;
   }

   TFn* func = const_cast<TFn*>(this);

   Double_t result = 0.0;
   if (fType == FORMULA) {
      result = func->fFormula.EvalPar(x,params);
   } else if (fType == FUNCTOR)  {
      if (!func->fFunctor.Empty()) result = func->fFunctor((Double_t*)x,(Double_t*)params);
      // else          result = const_cast<TFn*>(this)->GetSave(x);
   } else if (fType == INTERPRETER_FUNCTOR) {
      UpdateCintAddresses(x, params); 
      fMethodCall->Execute(result);
      // else             result = const_cast<TFn*>(this)->GetSave(x);
   }

   return result;
}


//______________________________________________________________________________
void TFn::FixParameter(UInt_t ipar, Double_t value)
{
   // Fix the value of a parameter
   // The specified value will be used in a fit operation

   if (ipar >= fNpar) {
      Error("FixParameter", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return; 
   }
   fParams[ipar] = value;

   if (value != 0.0) SetParLimits(ipar,value,value);
   else SetParLimits(ipar,1,1);
}


//______________________________________________________________________________
THn* TFn::GetHistogram(const UInt_t* npoints) const 
{
   // Create an approximative histogram for the function
  
   if (npoints == NULL) {
      Error("GetHistogram", "Array containting number of resolution points for each dimension has not been specified.");
      return NULL;
   } else {
      for (UInt_t i = 0; i < fNdim; ++i)
         if (npoints[i] == 0) {
            Error("GetHistogram", "Number of points to be extracted on dimension %u is set to 0. Invalid value", i);
            return NULL;
         }
   } 

   UInt_t cumulPoints[fNdim]; cumulPoints[fNdim - 1] = 1;
   for (UInt_t i = fNdim - 1; i > 0; --i) cumulPoints[i - 1] = cumulPoints[i] * npoints[i];
   UInt_t totalPoints = cumulPoints[0] * npoints[0];

   Double_t dx[fNdim]; // steps in changing x for each dimension, dependent on npoints
   Double_t origPoint[fNdim]; // origin point
   Double_t crtPoint[fNdim]; // placed in bin center
   UInt_t crtBin[fNdim]; // current bin index for each dimension

   for (UInt_t i = 0; i < fNdim; ++i) {
      dx[i] = (fMax[i] - fMin[i]) / npoints[i];
      origPoint[i] = fMin[i] + dx[i] / 2.0;
   }

   THn* hist = new THnD(TString::Format("%s_Histogram", GetName()).Data(), 
      TString::Format("THn Approximation of a %s", GetTitle()).Data(), fNdim, (const Int_t *)npoints, fMin, fMax);

   for (UInt_t i = 0; i < totalPoints; ++i) {
      // Get Current bin
      Int_t cumulBin = i;
      for (UInt_t j = 0; j < fNdim; ++j) {
         crtBin[j] = cumulBin / cumulPoints[j];
         cumulBin = cumulBin % cumulPoints[j];
         crtPoint[j] = origPoint[j] + dx[j] * crtBin[j];
      }
      R__ASSERT(cumulBin == 0);
       
      Int_t histBin = hist->GetBin(crtPoint); // NOTE: hist has different bin numbers because of underflow / overflow bins
      hist->SetBinContent(histBin, DoEvalPar(crtPoint, fParams));
   }

   return hist;
}


class TFn_ReverseSign : public ROOT::Math::IBaseFunctionMultiDim {
public:   
   TFn_ReverseSign(const TFn& func) : fFunc(func) {}
   virtual UInt_t NDim() const { return fFunc.NDim(); }
private:
   virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const { return NULL; }  
   const TFn& fFunc;  
   virtual Double_t DoEval(const Double_t* x) const { return -fFunc(x); }
};

//______________________________________________________________________________
Double_t* TFn::GetMaximumX(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxIter) const
{
   // Return the X value corresponding to the maximum value of the function on the [min, max] subdomain
   // If min, max are not set, the minimization is performed on the whole range
   // The user is responsible for deleting the array returned
   Double_t* x = new Double_t[fNdim];
   TFn_ReverseSign reverseSignFunc(*this);
   ConfigureAndMinimize(&reverseSignFunc, x, min, max, epsilon, maxIter);   
   return x;
}

//______________________________________________________________________________
Double_t* TFn::GetMinimumX(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxIter) const
{
   // Return the X value corresponding to the minimum value of the function on the [min, max] subdomain
   // If min, max are not set, the minimization is performed on the whole range
   // The user is responsible for deleting the array returned
   Double_t* x = new Double_t[fNdim];
   ConfigureAndMinimize(this, x, min, max, epsilon, maxIter);
   return x;
}


//______________________________________________________________________________
Double_t TFn::GetMaximum(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxIter) const
{
   // Returns the maximum value of the function on the [min, max] subdomain if present, else on the full range
   TFn_ReverseSign reverseSignFunc(*this);
   return -ConfigureAndMinimize(&reverseSignFunc, NULL, min, max, epsilon, maxIter);
}

//______________________________________________________________________________
Double_t TFn::GetMinimum(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxIter) const
{
   // Returns the minimum value of the function on the [min, max] subdomain if present, else on the full range
   return ConfigureAndMinimize(this, NULL, min, max, epsilon, maxIter);
}


//______________________________________________________________________________
Double_t TFn::ConfigureAndMinimize(const ROOT::Math::IBaseFunctionMultiDim* func, Double_t* x, Double_t* min, Double_t* max, Double_t epsilon, Int_t maxIter) const
{
   // Perform a N-dimensional minimization on the ranges min and man with precision epsilon using at most 'maxIter' iterations
   // The vector 'x' will contain the initial point on input, the minimum point on output;

   if (min == NULL) min = fMin; // if the input is not suitable, we bounce back to the object range minimum
   if (max == NULL) max = fMax; // if the input is not suitable, we bounce back to the object range maximum

   // fix invalid ranges
   for(UInt_t i = 0; i < fNdim; ++i) {
      if(min[i] >= max[i]) {
         min[i] = fMin[i];
         max[i] = fMax[i];
      }
   }

   Double_t *localX = NULL;
   if (x == NULL) {
      localX = new Double_t[fNdim]; // if no initial input point is provided, create one at the middle of the range
      for(UInt_t i = 0; i < fNdim; ++i) localX[i] = (min[i] + max[i]) / 2.0;
      x = localX;
   }

   // Create default minimizer
   const char* minimizerName = ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str();
   const char* minimizerAlgo = ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str();
   ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer(minimizerName, minimizerAlgo);
   R__ASSERT(minimizer != NULL);

   minimizer->SetFunction(*func);
   if(epsilon > 0) minimizer->SetTolerance(epsilon);
   if(maxIter > 0) minimizer->SetMaxFunctionCalls(100000);

   // set minimizer parameters (variables, step size, range)
   for (UInt_t i = 0; i < fNdim; ++i) {
      Double_t stepSize = 0.1;

      // use given argument range to determine the step size or give some value depending on x
      if (max[i] > min[i]) {
         stepSize = (max[i] - min[i]) / 100.0;
         minimizer->SetLimitedVariable(i, TString::Format("x[%d]", i).Data(), x[i], stepSize, min[i], max[i]);
      } else {
      // TODO: why? if(std::abs(x[i]) > 1.0)
         stepSize = 0.1 * x[i];
         minimizer->SetVariable(i, TString::Format("x[%d]", i).Data(), x[i], stepSize);
      }
   }

   // minimize and check success
   if (!minimizer->Minimize())
      Error("ConfigureAndMinimize", "Error minimizing function %s - check input and maximum number of iterations", GetName());
   else if(!minimizer->X())
      Error("ConfigureAndMinimize", "Minimizer did not return a valid minimum point (X array)");

   if (localX != NULL) delete localX; // no input vector was provided, nothing needs to be returned to the user
   else if (x != NULL) std::copy( minimizer->X(), minimizer->X() + fNdim, x); // set x to be point of minimum

   Double_t funcMin = minimizer->MinValue();
      
   delete minimizer;

   return funcMin;
}


//______________________________________________________________________________
UInt_t TFn::GetNumberFreeParameters() const
{
   // Return the number of free parameters

   UInt_t nfree = fNpar;
   Double_t al,bl;
   for (UInt_t i = 0; i < fNpar; ++i) {
      ((TFn*)this)->GetParLimits(i,al,bl);
      if (al*bl != 0 && al >= bl) nfree--;
   }
   return nfree;
}


//______________________________________________________________________________
Double_t TFn::GetParError(UInt_t ipar) const
{
   // Return value of parameter number ipar

   if (ipar >= fNpar) {
      Error("GetParError", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return 0.0;
   }
   return fParErrors[ipar];
}


//______________________________________________________________________________
void TFn::GetParLimits(UInt_t ipar, Double_t& parmin, Double_t& parmax) const
{
   // Return limits for parameter ipar.

   parmin = parmax = 0;
   if (ipar >= fNpar) {
      Error("GetParLimits", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return;
   }
   if (fParMin) parmin = fParMin[ipar];
   if (fParMax) parmax = fParMax[ipar];
}

//______________________________________________________________________________
void TFn::GetRange(Double_t*& min, Double_t*& max) const
{
   // Return range of a n-D function. Deletes input pointers.
   min = new Double_t[fNdim];
   max = new Double_t[fNdim];
   std::copy(fMin, fMin + fNdim, min);
   std::copy(fMax, fMax + fNdim, max);
}


//______________________________________________________________________________
void TFn::FdF(const Double_t* x, Double_t& f, Double_t* df) const {
   Gradient(x, df);
   f = DoEvalPar(x);
}

//______________________________________________________________________________
void TFn::Gradient(const Double_t* x, Double_t* grad) const
{
   // Compute the gradient (derivative) in a given n-dim point x
   // @in x = point where the derivative is computed
   // @out grad = gradient coefficients

   if (x == NULL) Error("Gradient", "Input parameter x missing");
   if (grad == NULL) {
      Warning("Gradient", "grad array not specified by user; creating internal %d-dim array; memory must be freed by user", fNdim);
   }

   for(UInt_t i = 0; i < fNdim; ++i)
      grad[i] = DoDerivative(x, i);
}

class TFn_OneVariableParameter : public ROOT::Math::IBaseFunctionOneDim {
public:
   TFn_OneVariableParameter(const TFn& func, const Double_t* x, const Double_t* params, UInt_t ipar) : fFunc(func), fIdxParam(ipar) {
      fX = new Double_t[func.NDim()];
      fParams = new Double_t[func.NPar()];
      std::copy(x, x + func.NDim(), fX);
      std::copy(params, params + func.NPar(), fParams);
   }
   ~TFn_OneVariableParameter() { delete [] fX; delete [] fParams; }

private:
   virtual TFn_OneVariableParameter* Clone() const { return NULL; } // do not use

   // params are the variable here, not the coordinates
   virtual Double_t DoEval(Double_t p) const {
      fParams[fIdxParam] = p;
      return fFunc(fX, fParams);
   }

   const TFn& fFunc;
   Double_t* fX;
   Double_t* fParams;
   const UInt_t fIdxParam;
};

Double_t TFn::DoParameterDerivative(const Double_t* x, const Double_t* params, UInt_t ipar) const 
{
   if (ipar >= fNpar) {
      Error("DoParameterDerivative", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return 0.0;
   }
   
   Double_t al, bl;
   GetParLimits(ipar, al, bl);
   if (al * bl != 0.0 && al >= bl) {
      Error("DoParameterDerivative", "Chosen input parameter with index %d is fixed", ipar);
      return 0.0;
   }

   const Double_t* actualParams = NULL;
   if (params == NULL) {
      Warning("DoParameterDerivative", "Input parameters not specified by user; using internal parameter values");
      actualParams = fParams;
   } else {
      actualParams = params;
   }
   
   TFn_OneVariableParameter func(*this, x, actualParams, ipar);
   ROOT::Math::RichardsonDerivator derivator(func);

   return derivator.Derivative1(actualParams[ipar]);
}


//______________________________________________________________________________
void TFn::UpdateCintAddresses(const Double_t *x, const Double_t *params) const
{
   // Initialize parameters addresses in case of CINT function.
   Long_t args[2];
   args[0] = (Long_t)x;
   if (params) args[1] = (Long_t)params;
   else        args[1] = (Long_t)fParams;
   fMethodCall->SetParamPtrs(args);
}


//______________________________________________________________________________
const Double_t* TFn::GetRandom() const { 
   /**
    * Return a random sample (vector of n-dim) from the multivariate distribution associated with
    * this function. In order to get the distribution, the function is normalised.
    */    
   if (!fSampler) {
      Double_t *min, *max; GetRange(min, max);
      TFn_Distribution* tn = new TFn_Distribution(*this); // FIXME maybe move to field status
      fSampler = ROOT::Math::Factory::CreateDistSampler(); 
      if (!fSampler) {
         Error("GetRandom", "ROOT::Math::Factory could not create the default ROOT::Math::DistSampler");
         return NULL;
      }
      fSampler->SetFunction(*tn); 
      fSampler->SetRange(min, max);
      fSampler->Init();
      delete min; delete max; // sampler clones range (apparently)
   }
   return fSampler->Sample();
}


/**
 * @class TFn_ParameterDerivative
 * @brief Auxiliary class used by TFn::IntegralError.
 *
 * Given a TFn representing a function f(x|p), where x is the input variable vector, p is the parameter 
 * vector, and p_i is the ith parameter, the user can create a new object equivalent to the function
 * df(x|p)/d{p_i} by wrapping the TFn in a TFn_ParameterDerivative.
 *
 */
class TFn_ParameterDerivative {
public:
   TFn_ParameterDerivative(const TFn& func, UInt_t ipar) : fFunc(func), fIdxParam(ipar) {}
   Double_t operator() (Double_t *x, Double_t*) const { return fFunc.ParameterDerivative(x, fIdxParam); }

   const TFn& fFunc;
   UInt_t fIdxParam;
};

//______________________________________________________________________________
Double_t TFn::IntegralError(const Double_t* a, const Double_t* b, const Double_t* params, const Double_t* covmat, Double_t eps)
{
   // Return Error on Integral of a parameteric function with dimension larger than one between a[] and b[],
   // due to the parameters uncertainties. TFn::IntegralMultiple is used for the integral calculation
   //
   // A pointer to the covariance matrix (covmat) must be passed.
   //
   // NOTE: The user must pass a pointer to the elements of the full covariance matrix dimensioned with the 
   // right size (npar*npar), where npar is the total number of parameters (TFn::GetNpar()), including 
   // fixed parameters.
   // One should use the TFitResult class, as shown in the example below.   
   // 
   // To get the matrix and values from an old fit do for example:  
   // TFitResultPtr r = histo->Fit(func, "S");
   // ..... after performing other fits on the same function do 
   // func->IntegralError(x1,x2,r->GetParams(), r->GetCovarianceMatrix()->GetMatrixArray() );

   if (fNpar == 0) {
      Error("IntegralError", "Function has no parameters");
      return 0.0;
   }
   if (covmat == NULL) {
      Error("IntegralError", "No input covariance matrix provided");
      return 0.0;
   }

   Double_t* oldParams = NULL;
   if (params) {
      oldParams = new Double_t[fNpar];
      std::copy(fParams, fParams + fNpar, oldParams);
      SetParameters(params);
   }

   TMatrixDSym covMatrix(fNpar);
   covMatrix.Use(fNpar, covmat);
   
   // loop on the parameter and calculate the errors
   TVectorD integrals(fNpar);
   for(UInt_t i = 0; i < fNpar; ++i) {
      // check that the parameter error is not zero, otherwise skip it
      integrals[i] = 0.0;
      if (covMatrix(i,i) > 0.0) {
         TFn gradFunc(TString::Format("%s_ParameterDerivative", GetName()),
            fNdim, TFn_ParameterDerivative(*this, i), fMin, fMax, fNpar);
         integrals[i] = gradFunc.Integral(a, b, eps, eps);
      }
   }
   Double_t err = covMatrix.Similarity(integrals); // integrals^T * covMatrix * integrals

   if (oldParams) SetParameters(oldParams);

   return std::sqrt(err);
}


//______________________________________________________________________________
Double_t TFn::Integral(const Double_t* a, const Double_t* b, Double_t absTol, Double_t relTol, UInt_t maxEval, Double_t* error, Int_t* status) const {
   /**
    * @brief Evaluate the multidimensional integral over a finite domain.
    *
    * @param[in] a = n-dim array containing the lower limits of integration; if NULL, function lower range is used
    * @param[in] b = n-dim array containing the upper limits of integration; if NULL, function upper range is used
    * @param[in] absTol = absolute tolerance of integration
    * @param[in] relTol = relative tolerance of integration
    * @param[in] maxEval = maximum number of function evaluations
    * @param[out] error = integration error
    * @param[out] status = integration error status
    * @return function integral value
    */

   ROOT::Math::IntegratorMultiDim integrator(*this, ROOT::Math::IntegrationMultiDim::kADAPTIVE, absTol, relTol, maxEval);

   if (a == NULL) a = fMin; // if lower bounds are not provided, use function range for calculating integral
   if (b == NULL) b = fMax; // if upper bounds are not provided, use function range for calculating integral
  
   Double_t result = integrator.Integral(a, b);
   if (error) *error = integrator.Error();
   if (status) *status = integrator.Status(); 

   return result;
}


//______________________________________________________________________________
Bool_t TFn::IsInside(const Double_t *x) const
{
   // Return kTRUE if the point is inside the function range

   for(UInt_t i = 0; i < fNdim; ++i)
      if (x[i] < fMin[i] || x[i] > fMax[i]) 
         return kFALSE;
   return kTRUE;
}


//______________________________________________________________________________
void TFn::Print(Option_t *option) const
{
   // TODO: improve
   TNamed::Print(option);
   Printf("Number of dimensions = %d, Number of parameters = %d", fNdim, fNpar);
   for (UInt_t i = 0; i < fNdim; ++i) Printf("Dimension %d range:\t [%12.6lf, %12.6lf]", i, fMin[i], fMax[i]);
}


//______________________________________________________________________________
void TFn::ReleaseParameter(UInt_t ipar)
{
   // Release parameter number ipar If used in a fit, the parameter
   // can vary freely. The parameter limits are reset to 0,0.
   if (ipar >= fNpar) {
      Error("ReleaseParameter", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return;
   }
   SetParLimits(ipar, 0.0, 0.0);
}

//______________________________________________________________________________
void TFn::SetParameters(const Double_t* params)
{
   // Set error for parameter number ipar
   if (params == NULL) {
      Warning("SetParameters", "Input parameter array is not allocated. Parameters have not changed.");
      return;
   }
   std::copy(params, params + fNpar, fParams);
}

//______________________________________________________________________________
void TFn::SetParError(UInt_t ipar, Double_t error)
{
   // Set error for parameter number ipar
   if (ipar >= fNpar) {
      Warning("SetParError", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return;
   }
   fParErrors[ipar] = error;
}


//______________________________________________________________________________
void TFn::SetParErrors(const Double_t *errors)
{
   // Set errors for all active parameters
   // when calling this function, the array errors must have at least fNpar values

   if (errors == NULL) {
      Warning("SetParErrors", "Input errors array was not specified; Keeping old parameter errors values");
      return;
   }
   for (UInt_t i = 0; i < fNpar; ++i) fParErrors[i] = errors[i];
}


//______________________________________________________________________________
void TFn::SetParLimits(UInt_t ipar, Double_t parmin, Double_t parmax)
{
   // Set limits for parameter ipar.
   // The specified limits will be used in a fit operation when the option "B" is specified (Bounds).
   // To fix a parameter, use TFn::FixParameter
   if (ipar >= fNpar) {
      Warning("SetParLimits", "Parameter index is out of range. Function has only %d parameters.", fNpar);
      return;
   }
   fParMin[ipar] = parmin; fParMax[ipar] = parmax;
}


//______________________________________________________________________________
void TFn::SetRange(Double_t* min, Double_t* max)
{
   // Initialize the upper and lower bounds to draw the function.
   //
   // The function range is also used in an histogram fit operation
   // when the option "R" is specified.
   if (min == NULL || max == NULL) {
      Warning("SetRange", "Input ranges have not been specified; keeping old values");
      return;
   }
   std::copy(min, min + fNdim, fMin);
   std::copy(max, max + fNdim, fMax);
}


/**
 * TFn_Projection1D is a helper / wrapper class used to fix one coordinate of the original
 * TFn and allow integration only on the other n-1 coordinates. 
 */
class TFn_Projection1D : public ROOT::Math::IBaseFunctionMultiDim {
public:
   // assumes correct arguments are passed
   TFn_Projection1D(const TFn& func, UInt_t icoord) :
      fFunc(func), fNdim(fFunc.NDim()), fIdxCoord(icoord), fValCoord(0.0), fIntegrator(*this)
   {
      fX = new Double_t[fNdim];

      fFunc.GetRange(fMin, fMax);
      // shift range for n-1 dim integral
      for(UInt_t i = fIdxCoord + 1; i < fNdim; ++i) {
         fMin[i - 1] = fMin[i];
         fMax[i - 1] = fMax[i];
      }
   }
   ~TFn_Projection1D() { delete [] fX; delete [] fMin; delete [] fMax; } 

   virtual UInt_t NDim() const { return fNdim - 1; }
   
   Double_t Integral(Double_t* valCoord, Double_t*) {
      fValCoord = valCoord[0];
      return fIntegrator.Integral(fMin, fMax);
   }
      
private:
   virtual TFn_Projection1D* Clone() const { return NULL; } // do not use !!!
      
   virtual Double_t DoEval(const Double_t *x) const {
      for(UInt_t i = 0; i < fIdxCoord; ++i) fX[i] = x[i];
      fX[fIdxCoord] = fValCoord;
      for(UInt_t i = fIdxCoord + 1; i < fNdim; ++i) fX[i] = x[i - 1];
      return fFunc(fX);
   }
     
   const TFn& fFunc; // function that is projected
   UInt_t fNdim; // dimension of TFn
   UInt_t fIdxCoord; // coordinate on which the projection is made
   mutable Double_t fValCoord; // value of the coordinate
   ROOT::Math::IntegratorMultiDim fIntegrator; // integrator used
   
   Double_t* fX;   //![fNdim] contains (n-1)-dim input array plus fixed value on specified coord
   Double_t* fMin; //![fNdim] contains original TFn min range
   Double_t* fMax; //![fNdim] contains original TFn max range
};

//______________________________________________________________________________
TF1* TFn::Projection1D(UInt_t icoord) const
{
   // Return the 1D projection of the function on the icoord coordonate
   // 
   // NOTE: The TFn and its projection are intrinsically linked; if one changes
   // (ranges for example), the other will as well

   if(fNdim == 1) {
      Error("Projection1D", "Trying to project a 1-D function on one dimension");
      return NULL;
   }
   if(icoord >= fNdim) {
      Error("Projection1D", "Coordonate index passed as input is out of dimensional range");
      return NULL;
   }
   // FIXME: remove const hack
   TFn_Projection1D* proj =  new TFn_Projection1D(*this, icoord);
   return new TF1(TString::Format("%s_Projection1D", GetName()), proj, &TFn_Projection1D::Integral,
      fMin[icoord], fMax[icoord], fNpar);
}


