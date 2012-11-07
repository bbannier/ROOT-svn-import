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


/**
 * @class TFn_Normalised 
 * @brief Auxiliary class used in TFn::Init to initialize fSampler.
 *
 * Given a TFn representing a function f(x|p), where x is the input variable vector, p is the parameter 
 * vector, and p_i is the ith parameter, the user can create a new object equivalent to f normalized
 * over the range of x, by wrapping the TFn in a TFn_Normalised.
 *
 */
class TFn_Normalised : public ROOT::Math::IBaseFunctionMultiDim {
public:
   TFn_Normalised(const TFn& func) : fFunc(func), fNormalisationConstant(fFunc.Integral()) {}

   UInt_t NDim() const { return fFunc.NDim(); }
   // NOTE: use with caution
   TFn_Normalised* Clone() const { return new TFn_Normalised(*fFunc.Clone()); }
  
private:
   const TFn& fFunc;
   const Double_t fNormalisationConstant;
   Double_t DoEval(const Double_t* x) const { return (*((TFn*)&fFunc))(x) / fNormalisationConstant; }
};



//______________________________________________________________________________
void TFn::Init(UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar) 
{
   // TFn initializer, employed by constructors
   fMin = fMax = NULL;
   fParams = fParErrors = fParMin = fParMax = NULL;
   fNorm = -1.0; // set an invalid value, until first calculation
   fNdim = fNpar = 0;

   if (ndim > 0) {
      fNdim = ndim; // XXX: should we put this in the initialization list?
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

      for (UInt_t i = 0; i < fNpar; i++)
         fParams[i] = fParErrors[i] = fParMin[i] = fParMax[i] = 0;
   }
   
   fSampler = ROOT::Math::Factory::CreateDistSampler();
   fSampler->SetFunction(TFn_Normalised(*this));
   if (!fSampler) {
      Error("Init", "ROOT::Math::Factory could not create the default ROOT::Math::DistSampler");
      return;
   }
 
} 

//______________________________________________________________________________
TFn::TFn() : 
   TNamed(), 
   fParent(NULL), 
   fType(EMPTY), 
   fFormula(NULL), 
   fFunctor(), 
   fCintFunc(NULL), 
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
   fFormula(new TFormula(name, formula)),
   fFunctor(),
   fCintFunc(NULL),
   fMethodCall(NULL)
{
   // TFn constructor using a formula definition.
   // See TFormula for the explanation of its constructor syntax.
   // See tutorials: fillrandom, first, fit1, formula1, multifit for real examples.

   if(!fFormula)
      Error("TFn::TFn", "object %s created incorrectly because of invalid formula", name);
   else
      Init(fFormula->GetNdim(), min, max, fFormula->GetNpar());
}

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar):
   TNamed(name, "TFn created from the name of an interpreted function"),
   fParent(NULL),
   fType(INTERPRETER_FUNCTOR),
   fFormula(NULL),
   fFunctor(),
   fCintFunc(NULL),
   fMethodCall(NULL),
   fSampler(NULL)
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
   // SetName(name);
   
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

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, void* fcn, Double_t* min, Double_t* max, UInt_t npar) :
   TNamed(name, "TFn created from a pointer to an interpreted function"),
   fParent(NULL),
   fType(INTERPRETER_FUNCTOR),
   fFormula(NULL),
   fFunctor(),
   fCintFunc(NULL),
   fMethodCall(NULL),
   fSampler(NULL)
{
   /**
    * TFn constructor using a pointer to an interpreted function.
    * This constructor is called for functions of type C by CINT.
    *
    * @param [in] min and max = ranges for the function variables
    * @param [in] npar = number of parameters with which the function is defined
    * @param [in] fcn = function of type "Double_t fcn(Double_t *x, Double_t *params)"
    *
    * see tutorial; myfit and test/stress.cxx
    *
    * WARNING! A function created with this constructor cannot be Cloned.
    */
   
   if (!fcn) {
      Error("TFn", "Input pointer to interpreted function is NULL - TFn object %s is incomplete", name);
      return;
   }

   Init(ndim, min, max, npar);

   TFn *fnOld = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(fnOld);
   SetName(name);

   const char *funcname = gCint->Getp2f2funcname(fcn);
   SetTitle(funcname);
   if (funcname) {
      fMethodCall = new TMethodCall();
      fMethodCall->InitWithPrototype(funcname, "Double_t*,Double_t*");
      gROOT->GetListOfFunctions()->Add(this);
      if (!fMethodCall->IsValid()) {
         Error("TFn", "No function found with the signature %s(Double_t*,Double_t*)", funcname);
      }
   } else {
      Error("TFn", "can not find any function at the address 0x%lx. This function requested for %s", (Long_t)fcn, name);
   }
}

/* XXX See if it can be removed
//______________________________________________________________________________
TFn::TFn(const char *name, UInt_t ndim, Double_t (*fcn)(Double_t *, Double_t *), Double_t* min, Double_t* max, UInt_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fParent(NULL),
   fType(FUNCTOR),
   fCintFunc(NULL),
   fFormula(NULL),
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
   ConfigureFunctor(name);
}

//______________________________________________________________________________
TFn::TFn(const char *name, UInt_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, UInt_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fParent(NULL),
   fType(FUNCTOR),
   fCintFunc(NULL),
   fFormula(NULL),
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
//   ConfigureFunctor(name);
}
*/

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, UInt_t npar ) :
   TNamed(name, "TFn created from ROOT::Math::ParamFunctor"),
   fParent(NULL),
   fType(FUNCTOR),
   fFormula(NULL),
   fFunctor(f),
   fCintFunc(NULL),
   fMethodCall(NULL),
   fSampler(NULL)
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
   Init(ndim, min, max, npar);
   ConfigureFunctor(name);
}


//______________________________________________________________________________
void TFn::ConfigureFunctor(const char* name)
{
   // Internal Function to Create a TFn  using a Functor.
   // Store formula in linked list of formula in ROOT
   // TODO: make this work, or remove if not really needed
   TFn *fnOld = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   if(fnOld) gROOT->GetListOfFunctions()->Remove(fnOld);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);
}


//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, void* ptr, Double_t* min, Double_t* max, UInt_t npar, const char* className) : 
   TNamed(name, TString::Format("CINT - class %s", className).Data()),
   fParent(),
   fType(INTERPRETER_CLASS),
   fFormula(NULL),
   fFunctor(),
   fCintFunc(NULL),
   fMethodCall(NULL),
   fSampler(NULL)
{
   /**
    * @brief TFn constructor from an interpreted class defining the operator() or Eval().
    * 
    * This constructor emulates the syntax of the template constructor using a C++
    * callable object (functor), which can only be used in C++ compiled code.
    *
    * The class name is required to get the type of class given the void pointer ptr.
    * For the method name is used the operator() (Double_t *, Double_t * ).
    * Use the other constructor taking the method name for different method names.
    *
    * @param[in] min  The lower bounds of vector coordinates
    * @param[in] max  The upper bounds of vector coordinates
    * @param[in] npar The number of function parameters
    *
    * @example tutorials/math/exampleFunctor.C
    * @warning This constructor is used only when using CINT.
    */
   Init(ndim, min, max, npar);
   ConfigureCintClass(name, ptr, className, NULL);
}

//______________________________________________________________________________
TFn::TFn(const char* name, UInt_t ndim, void *ptr, void *, Double_t* min, Double_t* max, UInt_t npar, const char* className, const char* methodName) : 
   TNamed(name, TString::Format("CINT - class %s - method %s", className, methodName).Data()),
   fParent(),
   fType(INTERPRETER_CLASS),
   fFormula(NULL),
   fFunctor(),
   fCintFunc(NULL),
   fMethodCall(NULL),
   fSampler(NULL)
{
   /**
    * @brief TFn constructor from an interpreter class using a specified member function.
    *
    * This constructor emulates the syntax of the template constructor using a C++ class 
    * and a given member function pointer, which can only be used in C++ compiled mode.
    *
    * The class name is required to get the type of class given the void pointer ptr.
    * The second void * is not needed for the CINT case, but is kept for emulating the API of the template constructor.
    * The method name is optional. By default is looked for operator() (Double_t *, Double_t *) or Eval(Double_t *, Double_t*)
    *
    * @param[in] min  The lower bounds of vector coordinates
    * @param[in] max  The upper bounds of vector coordinates
    * @param[in] npar The number of function parameters
    *
    * @example tutorials/math/exampleFunctor.C
    * @warning This constructor is used only when using CINT.
    */
   Init(ndim, min, max, npar);
   ConfigureCintClass(name, ptr, className, methodName);
}

//______________________________________________________________________________
void TFn::ConfigureCintClass(const char *name, void *ptr, const char * className, const char* methodName)
{
   // Internal function used to create from TFn from an interpreter CINT class
   // with the specified type (className) and member function name (methodName).

   TFn *fnOld = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(fnOld);
   SetName(name);

   if (!ptr) return;
   fCintFunc = ptr;

   if (!className) return;
   TClass *cl = TClass::GetClass(className);

   if (cl) {
      fMethodCall = new TMethodCall();

      if (methodName) 
         fMethodCall->InitWithPrototype(cl, methodName, "Double_t*,Double_t*");
      else {
         fMethodCall->InitWithPrototype(cl, "operator()", "Double_t*,Double_t*");
         if (! fMethodCall->IsValid() ) // try with Eval if operator() is not found
            fMethodCall->InitWithPrototype(cl,"Eval","Double_t*,Double_t*");
      }

      gROOT->GetListOfFunctions()->Add(this);
      if (! fMethodCall->IsValid() ) {
         if (methodName)
            Error("TFn", "No function found in class %s with the signature %s(Double_t*,Double_t*)", className, methodName);
         else
            Error("TFn", "No function found in class %s with the signature operator() (Double_t*,Double_t*) or Eval(Double_t*,Double_t*)", className);
      }
   } else {
      Error("TFn", "cannot find any class with name %s at the address 0x%lx", className, (Long_t)ptr);
   }
}


//______________________________________________________________________________
TFn::~TFn()
{
   // TFn destructor.

   if (gROOT) gROOT->GetListOfFunctions()->Remove(this);
   if (fParent) fParent->RecursiveRemove(this); fParent = NULL; 

   delete [] fMin;
   delete [] fMax;
   delete [] fParams;
   delete [] fParMin;
   delete [] fParMax;
   delete [] fParErrors;
   delete fFormula;
   delete fMethodCall;
   delete fSampler;
}


//______________________________________________________________________________
TFn::TFn(const TFn &rhs, const char* name) : TNamed(rhs) 
{
   // TFn copy constructor.
   if (name) SetName(name);
   else SetName(TString::Format("%s_copy", rhs.GetName()));

   fType = rhs.fType; 
   fFormula = rhs.fFormula; 
   fCintFunc = rhs.fCintFunc;
   fMethodCall = rhs.fMethodCall;
   fParent = rhs.fParent;

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
      fCintFunc = rhs.fCintFunc;
      delete fFormula; if(rhs.fFormula) fFormula = new TFormula(*fFormula);
      delete fMethodCall; if (rhs.fMethodCall) fMethodCall =  new TMethodCall(*fMethodCall);

      if (fNdim != rhs.fNdim) {
         fNdim = rhs.fNdim;
         delete [] fMin; 
         delete [] fMax;

         if(fNdim > 0) {
            fMin = new Double_t[fNdim];
            fMax = new Double_t[fNdim];
            std::copy(rhs.fMin, rhs.fMin + fNdim, fMin);
            std::copy(rhs.fMax, rhs.fMax + fNdim, fMax);
         }
      }

      if (fNpar != rhs.fNpar) {
         fNpar = rhs.fNpar;
         delete [] fParMin; 
         delete [] fParMax; 
         delete [] fParErrors;
        
         if (fNpar > 0) { 
            fParMin = new Double_t[fNpar];
            fParMax = new Double_t[fNpar];
            fParErrors = new Double_t[fNpar];
            std::copy(rhs.fParMin, rhs.fParMin + fNpar, fParMin);
            std::copy(rhs.fParMax, rhs.fParMax + fNpar, fParMax);
            std::copy(rhs.fParErrors, rhs.fParErrors + fNpar, fParErrors);
         }
      }
   }

   return *this;
}

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

   Double_t* xm = const_cast<Double_t*>(x); 
   Double_t  x0 = x[icoord];
   
   // The step size is chosen depending on x[icoord], except when the latter is too small, in which case
   // a default epsilon is used.
   Double_t eps = std::numeric_limits<Double_t>::epsilon() / 7e-7; // (see R numDeriv package)
   Double_t h = 1e-4 * x0 < eps ? eps : 1e-4 * x0; // STEP SIZE -> gives precision
   
   xm[icoord] = x0 + h; Double_t f_plus  = DoEvalPar(xm);
   xm[icoord] = x0 - h; Double_t f_minus = DoEvalPar(xm);
   Double_t diff_h = f_plus - f_minus; 
   // So far we have O(h^2) for the error term

   // Now we use 2*h instead of h
   h *= 2.0;

   xm[icoord] = x0 + h;          f_plus  = DoEvalPar(xm);
   xm[icoord] = x0 - h;          f_minus = DoEvalPar(xm);
   Double_t diff_2h = f_plus - f_minus;

   xm[icoord] = x0; // restore initial value of vector (it has virtually remained const)

   // Richardson's extrapolation for f'(x), simplified expression -> O(h^3) error term
   // Solution is equivalent to the five-point stencil (http://en.wikipedia.org/wiki/Five-point_stencil)
   // note that the 6.0 appears instead of 12.0 at the denominator because we have doubled 'h'
   return (8.0 * diff_h - diff_2h) / (6.0 * h);
}


//______________________________________________________________________________
Double_t TFn::DoEvalPar(const Double_t *x, const Double_t *params) const
{
   // Evaluate function with given coordinates and parameters.
   //
   // Compute the value of this function at point defined by array x
   // and current values of parameters in array params.
   // If argument params is omitted, the internal values of parameters
   // (array fParams) will be used instead.
   // ! x must be filled with the corresponding number of dimensions.
   //
   // XXX: analyse this
   // WARNING. In case of an interpreted function (fType = INTERPRETER_FUNCTOR), 
   // it is the user's responsibility to initialize the parameters via InitArgs
   // before calling this function. InitArgs should be called at least once to 
   // specify the addresses of the arguments x and params. InitArgs should be called 
   // everytime these addresses change.

   TFn* func = const_cast<TFn*>(this);
   if(fMethodCall) func->InitArgs(x, params); // TODO: yes or not?

   Double_t result = 0.0;
   if (fType == FORMULA) {
      assert(fFormula != NULL);
      return fFormula->EvalPar(x,params);
   } else if (fType == FUNCTOR)  {
      if (!func->fFunctor.Empty()) {
         if (params) result = func->fFunctor((Double_t*)x,(Double_t*)params);
         else        result = func->fFunctor((Double_t*)x,fParams);
      } // else          result = const_cast<TFn*>(this)->GetSave(x);
   } else if (fType == INTERPRETER_FUNCTOR) {
      if (fMethodCall) fMethodCall->Execute(result);
      // else             result = const_cast<TFn*>(this)->GetSave(x);
   } else if (fType == INTERPRETER_CLASS) {
      if (fMethodCall) fMethodCall->Execute(fCintFunc,result);
      // else             result = const_cast<TFn*>(this)->GetSave(x);
   }
   return result;
}


//______________________________________________________________________________
void TFn::FixParameter(UInt_t ipar, Double_t value)
{
   // Fix the value of a parameter
   // The specified value will be used in a fit operation

   if (ipar >= fNpar) return; 
   fParams[ipar] = value;

   if (value != 0.0) SetParLimits(ipar,value,value);
   else SetParLimits(ipar,1,1);
}


//______________________________________________________________________________
THn* TFn::GetHistogram() const 
{
   // Create an approximative histogram for the function
  // TODO: points 
   Int_t* fNpoints = new Int_t[fNdim];
   Double_t* df = new Double_t[fNdim];
   Double_t* crtPoint = new Double_t[fNdim]; // placed in bin center
   for (UInt_t i = 0; i < fNdim; ++i) {
      fNpoints[i] = 10;
      df[i] = (fMax[i] - fMin[i]) / fNpoints[i];
      crtPoint[i] = fMin[i] - df[i] / 2.0;
   }

   THn* hist = new THnD(TString::Format("%s_Histogram", GetName()).Data(), 
      TString::Format("%s THn Approximation", GetTitle()).Data(), fNdim, fNpoints, fMin, fMax);

   for (UInt_t i = 0; i < fNdim; ++i) {
      for (Int_t j = 0; j < fNpoints[i]; ++j) {
         crtPoint[i] += df[i]; // traverse the grid
         Int_t bin = hist->GetBin(crtPoint);
         hist->SetBinContent(bin, DoEvalPar(crtPoint, fParams));
      }
   }

   return hist;
}



class TFn_ReverseSign : public ROOT::Math::IBaseFunctionMultiDim {
public:   
   TFn_ReverseSign(TFn* func) : fFunc(func) {}
   virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const {
      return new TFn_ReverseSign(fFunc);
   }  
   virtual UInt_t NDim() const { return fFunc->NDim(); }
private:
   TFn* fFunc;  
   virtual Double_t DoEval(const Double_t* x) const {
      return -(*fFunc)(x);
   }
};

//______________________________________________________________________________
Double_t* TFn::GetMaximumX(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter) const
{
   // Return the X value corresponding to the maximum value of the function on the [min, max] subdomain
   // If min, max are not set, the minimization is performed on the whole range
   // The user is responsible for deleting the array returned
   Double_t* x = new Double_t[fNdim];
   TFn_ReverseSign reverseSignFunc(const_cast<TFn*>(this));
   ConfigureAndMinimize(&reverseSignFunc, x, min, max, epsilon, maxiter);   
   return x;
}

//______________________________________________________________________________
Double_t* TFn::GetMinimumX(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter) const
{
   // Return the X value corresponding to the minimum value of the function on the [min, max] subdomain
   // If min, max are not set, the minimization is performed on the whole range
   // The user is responsible for deleting the array returned
   Double_t* x = new Double_t[fNdim];
   ConfigureAndMinimize(const_cast<TFn*>(this), x, min, max, epsilon, maxiter);
   return x;
}


//______________________________________________________________________________
Double_t TFn::GetMaximum(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter) const
{
   // Returns the maximum value of the function on the [min, max] subdomain if present, else on the full range
   TFn_ReverseSign reverseSignFunc(const_cast<TFn*>(this));
   return -ConfigureAndMinimize(&reverseSignFunc, NULL, min, max, epsilon, maxiter);
}

//______________________________________________________________________________
Double_t TFn::GetMinimum(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter) const
{
   // Returns the minimum value of the function on the [min, max] subdomain if present, else on the full range
   return ConfigureAndMinimize(const_cast<TFn*>(this), NULL, min, max, epsilon, maxiter);
}


//______________________________________________________________________________
Double_t TFn::ConfigureAndMinimize(ROOT::Math::IBaseFunctionMultiDim* func, Double_t* x, Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter) const
{
   // Perform a N-dimensional minimization on the ranges min and man with precision epsilon using at most 'maxiter' iterations
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
   if(maxiter > 0) minimizer->SetMaxFunctionCalls(maxiter);

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

   // TODO: remove in the end
   minimizer->SetPrintLevel(1);

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
      Error("GetParError", "Parameter index is out of dimensional range");
      return 0.0;
   }
   return fParErrors[ipar];
}


//______________________________________________________________________________
void TFn::GetParLimits(UInt_t ipar, Double_t &parmin, Double_t &parmax) const
{
   // Return limits for parameter ipar.

   parmin = parmax = 0;
   if (ipar >= fNpar) return;
   if (fParMin) parmin = fParMin[ipar];
   if (fParMax) parmax = fParMax[ipar];
}

//______________________________________________________________________________
void TFn::GetRange(Double_t *min, Double_t *max) const
{
   // Return range of a n-D function.
   // NOTE: the user is responsible for creating arrays of sufficient size
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

Double_t TFn::DoParameterDerivative(const Double_t* x, const Double_t* p, UInt_t ipar) const 
{
   
   if (ipar >= fNpar) {
      Warning("DoParameterDerivative", "Input parameter index is out of dimensional range; returning 0.0");
      return 0.0;
   }

   Double_t* params = new Double_t[fNpar];

   if (p == NULL) {
      Warning("DoParameterDerivative", "Input parameters not specified by user; using internal parameter values");
      std::copy(fParams, fParams + fNpar, params);
   } else {
      std::copy(p, p + fNpar, params);
   }

   Double_t h; Double_t eps = 1e-6; // TODO: make eps configurable somehow
   TFn *func = (TFn*)this;

   //save original parameters
   Double_t par0 = params[ipar];

   func->InitArgs(x, params);

   Double_t al, bl;
   Double_t f1, f2, g1, g2, h2, d0, d2;

   func->GetParLimits(ipar, al, bl);
   if (al * bl != 0.0 && al >= bl) {
      //this parameter is fixed
      return 0;
   }

   // check if error has been computer (is not zero)
   if (func->GetParError(ipar) != 0.0)
      h = eps * func->GetParError(ipar);
   else
      h = eps;

   params[ipar] = par0 + h;     f1 = func->DoEvalPar(x, params);
   params[ipar] = par0 - h;     f2 = func->DoEvalPar(x, params);
   params[ipar] = par0 + h/2;   g1 = func->DoEvalPar(x, params);
   params[ipar] = par0 - h/2;   g2 = func->DoEvalPar(x, params);

   //compute the central differences
   h2 = 1.0 / (2.0 * h);
   d0 = f1 - f2;
   d2 = 2.0 * (g1 - g2);

   Double_t grad = h2 * (4.0 * d2 - d0) / 3.0;

   // restore original value
   fParams[ipar] = par0;

   return grad;
}

/*
//______________________________________________________________________________
Double_t TFn::GradientPar(UInt_t ipar, const Double_t *x, Double_t eps) const
{
   // Compute the gradient (derivative) wrt a parameter ipar
   // Parameters:
   // ipar - index of parameter for which the derivative is computed
   // x - point, where the derivative is computed
   // eps - if the errors of parameters have been computed, the step used in
   // numerical differentiation is eps*parameter_error.
   // if the errors have not been computed, step=eps is used
   // default value of eps = 0.01
   // Method is the same as in Derivative() function
   //
   // If a paramter is fixed, the gradient on this parameter = 0

   if (fNpar == 0) return 0.0; 

   if (eps< 1e-10 || eps > 1) {
      Warning("Derivative","parameter esp=%g out of allowed range[1e-10,1], reset to 0.01", eps);
      eps = 0.01;
   }

   Double_t h;
   TFn *func = (TFn*)this;

   //save original parameters
   Double_t par0 = fParams[ipar];

   func->InitArgs(x, fParams);

   Double_t al, bl;
   Double_t f1, f2, g1, g2, h2, d0, d2;

   ((TFn*)this)->GetParLimits(ipar,al,bl);
   if (al*bl != 0 && al >= bl) {
      //this parameter is fixed
      return 0;
   }

   // check if error has been computer (is not zero)
   if (func->GetParError(ipar) != 0.0)
      h = eps*func->GetParError(ipar);
   else
      h = eps;

   fParams[ipar] = par0 + h;     f1 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 - h;     f2 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 + h/2;   g1 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 - h/2;   g2 = func->DoEvalPar(x,fParams);

   //compute the central differences
   h2 = 1 / (2.0 * h);
   d0 = f1 - f2;
   d2 = 2.0 * (g1 - g2);

   Double_t  grad = h2*(4*d2 - d0)/3.;

   // restore original value
   fParams[ipar] = par0;

   return grad;
}


//______________________________________________________________________________
void TFn::GradientPar(const Double_t *x, Double_t *grad, Double_t eps) const
{
   
    * Compute the gradient with respect to the parameters (fParams);
    * @param[in] x - point, were the gradient is computed
    * @param[in] eps = 0.01 - step used in numerical differentiation;
    *    if the parameter errors have been computed, the step used is
    *    eps * parError instead of eps
    * @param[out] grad - array where the computed gradient is returned, assumed to be of size fNpar
    *
    * The differentiation method employed is the same as in Derivative().
    * If a parameter is fixed, the gradient on it is 0.0 . 
    
    
   if(eps< 1e-10 || eps > 1) {
      Warning("Derivative","parameter esp=%g out of allowed range[1e-10,1], reset to 0.01",eps);
      eps = 0.01;
   }

   for (UInt_t ipar = 0; ipar < fNpar; ++ipar)
      grad[ipar] = GradientPar(ipar, x, eps);
}
*/

//______________________________________________________________________________
void TFn::InitArgs(const Double_t *x, const Double_t *params)
{
   // Initialize parameters addresses.

   if (fMethodCall) {
      Long_t args[2];
      args[0] = (Long_t)x;
      if (params) args[1] = (Long_t)params;
      else        args[1] = (Long_t)fParams;
      fMethodCall->SetParamPtrs(args);
   }
}


//______________________________________________________________________________
const Double_t* TFn::GetRandom() const { 
   /**
    * Return a random sample (vector of n-dim) from the multivariate distribution associated with
    * this function. In order to get the distribution, the function is normalised.
    */ 
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
   TFn_ParameterDerivative(UInt_t ipar, TFn& func) : fFunc(func), fIdxParam(ipar) {}
   Double_t operator() (Double_t *x, Double_t*) const { return fFunc.ParameterDerivative(x, fIdxParam); }

   TFn& fFunc;
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
         TFn gradFunc(TString::Format("%s_GradientPar", GetName()),
            fNdim, TFn_ParameterDerivative(i, *(TFn*)this), 0, 0, 0);
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

   for(UInt_t i = 0; i < fNdim; i++)
      if (x[i] < fMin[i] || x[i] > fMax[i]) 
         return kFALSE;

   return kTRUE;
}


//______________________________________________________________________________
void TFn::Print(Option_t *option) const
{
   // TODO: improve
   TNamed::Print(option);
   Printf(" %20s: %s Ndim = %d, Npar = %d", GetName(), GetTitle(), fNdim, fNpar);
}


//______________________________________________________________________________
void TFn::ReleaseParameter(UInt_t ipar)
{
   // Release parameter number ipar If used in a fit, the parameter
   // can vary freely. The parameter limits are reset to 0,0.

   if (ipar >= fNpar) return;
   SetParLimits(ipar, 0.0, 0.0);
}


//______________________________________________________________________________
void TFn::SetParError(UInt_t ipar, Double_t error)
{
   // Set error for parameter number ipar

   if (ipar >= fNpar) return;
   fParErrors[ipar] = error;
}


//______________________________________________________________________________
void TFn::SetParErrors(const Double_t *errors)
{
   // Set errors for all active parameters
   // when calling this function, the array errors must have at least fNpar values

   if (!errors) return;
   for (UInt_t i = 0; i < fNpar; ++i) fParErrors[i] = errors[i];
}


//______________________________________________________________________________
void TFn::SetParLimits(UInt_t ipar, Double_t parmin, Double_t parmax)
{
   // Set limits for parameter ipar.
   //  The specified limits will be used in a fit operation when the option "B" is specified (Bounds).
   // To fix a parameter, use TFn::FixParameter

   if (ipar >= fNpar) return;
   fParMin[ipar] = parmin;
   fParMax[ipar] = parmax;
}


//______________________________________________________________________________
void TFn::SetRange(Double_t* min, Double_t* max)
{
   // Initialize the upper and lower bounds to draw the function.
   //
   // The function range is also used in an histogram fit operation
   // when the option "R" is specified.
   if (min == NULL || max == NULL) {
      Error("SetRange", "Input ranges are invalid");
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
   // TODO: constify !!!
   TFn_Projection1D(TFn& func, UInt_t icoord) :
      fFunc(func), fNdim(fFunc.NDim()), fIdxCoord(icoord), fValCoord(0.0), fIntegrator(*this)
   {
      fX = new Double_t[fNdim];
      fMin = new Double_t[fNdim];
      fMax = new Double_t[fNdim];

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
      // TODO: check if in range
      fValCoord = valCoord[0];
      return fIntegrator.Integral(fMin, fMax);
   }
      
protected:
   // NOTE: DO NOT USE !!! Shallow copying
   virtual TFn_Projection1D* Clone() const { return new TFn_Projection1D(fFunc, fIdxCoord); } 
   
private:
   virtual Double_t DoEval(const Double_t *x) const {
      R__ASSERT(x != NULL);
      for(UInt_t i = 0; i < fIdxCoord; ++i) fX[i] = x[i];
      fX[fIdxCoord] = fValCoord;
      for(UInt_t i = fIdxCoord + 1; i < fNdim; ++i) fX[i] = x[i - 1]; 
      return fFunc(fX);
   }
     
   TFn& fFunc; // function that is projected
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
   TFn_Projection1D* proj =  new TFn_Projection1D(*(TFn*)this, icoord);
   return new TF1(TString::Format("%s_Projection1D", GetName()), proj, &TFn_Projection1D::Integral,
      fMin[icoord], fMax[icoord], fNpar);
}


