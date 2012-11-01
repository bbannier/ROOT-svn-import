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
#include "TFn.h"
#include "TH1.h"
#include "TGraph.h"
#include "TVirtualPad.h"
#include "TStyle.h"
#include "TRandom.h"
#include "TInterpreter.h"
#include "TPluginManager.h"
#include "TBrowser.h"
#include "TColor.h"
#include "TClass.h"
#include "TMethodCall.h"
#include "TF1Helper.h"
#include "Math/WrappedFunction.h"
#include "Math/WrappedTF1.h"
#include "Math/AdaptiveIntegratorMultiDim.h"
#include "Math/BrentRootFinder.h"
#include "Math/BrentMinimizer1D.h"
#include "Math/BrentMethods.h"
#include "Math/Factory.h"
#include "Math/GaussIntegrator.h"
#include "Math/GaussLegendreIntegrator.h"
#include "Math/Integrator.h"
#include "Math/Minimizer.h"
#include "Math/RichardsonDerivator.h"
#include "Math/Functor.h"
#include "Fit/FitResult.h"

//#include <iostream>


ClassImp(TFn)

// class wrapping function evaluation directly in n-Dim interface (used for integration) 
// and implementing the methods for the momentum calculations
/*
class  TFn_EvalWrapper : public ROOT::Math::IParametricGradFunctionMultiDim { 
public: 
   // TODO: remove from here
   //   TFn_EvalWrapper func(this, params, kTRUE, n); Moments and CentralMoments norm
   //
   TFn_EvalWrapper(TFn * f, const Double_t * par, bool useAbsVal, Double_t n = 1, Double_t x0 = 0) : 
      fFunc(f), 
      // TODO: fix this, or maybe not use at all
      fPar(par),
//      fPar( ( (par) ? par : f->GetParameters() ) ),
      fAbsVal(useAbsVal),
      fN(n), 
      fX0(x0)   
   {
      fFunc->InitArgs(fX, fPar); 
   }

   ROOT::Math::IGenFunction * Clone()  const { 
      // use default copy constructor
      TFn_EvalWrapper * f =  new TFn_EvalWrapper( *this);
      f->fFunc->InitArgs(f->fX, f->fPar); 
      return f;
   }
   // evaluate |f(x)|
   Double_t DoEval( Double_t* x) const {  
      Double_t fval = fFunc->DoEvalPar(x, fPar);
      return (fAbsVal && fval < 0) ? -fval : fval;
   } 
   // evaluate x * |f(x)|
   Double_t EvalFirstMom( Double_t x) { 
      fX[0] = x; 
      return fX[0] * TMath::Abs( fFunc->DoEvalPar( fX, fPar) ); 
   } 
   // evaluate (x - x0) ^n * f(x)
   Double_t EvalNMom( Double_t x) const  { 
      fX[0] = x; 
      return TMath::Power( fX[0] - fX0, fN) * TMath::Abs( fFunc->DoEvalPar( fX, fPar) ); 
   }

   TFn * fFunc; 
   mutable Double_t fX[1]; 
   const Double_t * fPar; 
   Bool_t fAbsVal;
   Double_t fN; 
   Double_t fX0;
};*/

//______________________________________________________________________________
/* Begin_Html
<center><h2>TFn: n-Dim function class</h2></center>
A TFn object is a n-Dim function defined between a lower and upper limit.
<br>The function may be a simple function (see <tt>TFormula</tt>) or a
precompiled user function.
<br>The function may have associated parameters.
<br>TFn graphics function is via the <tt>TH1/TGraph</tt> drawing functions.
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
Parameters may be given a name:
<pre>
   fa->SetParName(0,"Constant");
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
Consider the macro myfunc.C below:
<div class="code"><pre>
   // Macro myfunc.C
   Double_t myfunction(Double_t *x, Double_t *par)
   {
      Float_t xx =x[0];
      Double_t f = TMath::Abs(par[0]*sin(par[1]*xx)/xx);
      return f;
   }
   void myfunc()
   {
      TFn *f1 = new TFn("myfunc",myfunction,0,10,2);
      f1->SetParameters(2,1);
      f1->SetParNames("constant","coefficient");
      f1->Draw();
   }
   void myfit()
   {
      TH1F *h1=new TH1F("h1","test",100,0,10);
      h1->FillRandom("myfunc",20000);
      TFn *f1=gROOT->GetFunction("myfunc");
      f1->SetParameters(800,1);
      h1->Fit("myfunc");
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

End_Html
Begin_Html

<tt>TFn</tt> objects can reference other <tt>TFn</tt> objects (thanks John
Odonnell) of type A or B defined above. This excludes CINT interpreted functions
and compiled functions. However, there is a restriction. A function cannot
reference a basic function if the basic function is a polynomial polN.
<p>Example:
<div class="code"><pre>
   {
      TFn *fcos = new TFn ("fcos", "[0]*cos(x)", 0., 10.);
      fcos->SetParNames( "cos");
      fcos->SetParameter( 0, 1.1);

      TFn *fsin = new TFn ("fsin", "[0]*sin(x)", 0., 10.);
      fsin->SetParNames( "sin");
      fsin->SetParameter( 0, 2.1);

      TFn *fsincos = new TFn ("fsc", "fcos+fsin");

      TFn *fs2 = new TFn ("fs2", "fsc+fsc");
   }
</pre></div><div class="clear" />

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
class  MyFunction {
 public:
   ...
   Double_t Evaluate() (Double_t *x, Double_t *p) {
      // function implementation
   }
};
{
    ....
   MyFunction * fptr = new MyFunction(....);  // create the user function class
   TFn * f = new TFn("f",fptr,&MyFunction::Evaluate,0,1,npar,"MyFunction","Evaluate");   // create TFn class.

   .....
}
</pre></div><div class="clear" />
When constructing the TFn class, the name of the function class and of the member function are required only
if running in CINT and they are not need in compiled C++ mode.
See also the tutorial math/exampleFunctor.C for a running example.

End_Html */



//______________________________________________________________________________
void TFn::Init(UInt_t ndim, Double_t* min, Double_t* max, UInt_t npar) 
{
   // TFn initializer, employed by constructors
   fMin = fMax = NULL;
   fParams = fParErrors = fParMin = fParMax = NULL;
   fNorm = -1.0; // set an invalid value, until first calculation

   if (ndim > 0) {
      fNdim = ndim; // XXX: should we put this in the initialization list?
      fMin = new Double_t[fNdim];
      fMax = new Double_t[fNdim];
      std::copy(min, min + fNdim, fMin);
      std::copy(max, max + fNdim, fMax);
      
      fIntegrator.SetFunction(*this);
   
      // TODO: calculate in Norm()
   }   
   
   if (npar > 0) {
      fNpar = npar;
      //fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];

      for (UInt_t i = 0; i < fNpar; i++)
         fParams[i] = fParErrors[i] = fParMin[i] = fParMax[i] = 0;
   }
 
} 

//______________________________________________________________________________
TFn::TFn() : TNamed(), fIntegrator(), fType(0), fParent(NULL), fMethodCall(NULL), fCintFunc(NULL)
{
   // TFn default constructor.
   Init(0, NULL, NULL, 0);
}

//______________________________________________________________________________
TFn::TFn(const char* name, const char* formula, Double_t* min, Double_t* max) :
   TNamed(name, "TFn created from a formula definition (through TFormula)"),
   fIntegrator()
{
   // Constructor using a formula definition.
   // See TFormula for the explanation of the constructor syntax.
   // See tutorials: fillrandom, first, fit1, formula1, multifit for real examples.
   fFormula = new TFormula(TString::Format("%s_formula", name).Data(), formula);
   if(!fFormula) {
      Error("TFn::TFn", "object %s created incorrectly because of invalid formula", name);
   } else {

      Init(fFormula->GetNdim(), min, max, fFormula->GetNpar());
      fType      = 0;
      fParent     = 0;
      fMethodCall = 0;
      fCintFunc   = 0;
   }

}

//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, void* fcn, Double_t* min, Double_t* max, Int_t npar) :
   TNamed(name, "TFn created from a pointer to an interpreted function"),
   fIntegrator()
{
   // F1 constructor using pointer to an interpreted function.
   //
   //  See TFormula constructor for explanation of the formula syntax.
   //
   //  Creates a function of type C between xmin and xmax.
   //  The function is defined with npar parameters
   //  fcn must be a function of type:
   //     Double_t fcn(Double_t *x, Double_t *params)
   //
   //  see tutorial; myfit for an example of use
   //  also test/stress.cxx (see function stress1)
   //
   //
   //  This constructor is called for functions of type C by CINT.
   //
   //  WARNING! A function created with this constructor cannot be Cloned.

   Init(ndim, min, max, npar);

   fType       = 2;
   //fFunction   = 0;
  fParent     = 0;
   fMethodCall = 0;
   fCintFunc   = 0;

   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);


   if (!fcn) return;
   const char *funcname = gCint->Getp2f2funcname(fcn);
   SetTitle(funcname);
   if (funcname) {
      fMethodCall = new TMethodCall();
      fMethodCall->InitWithPrototype(funcname,"Double_t*,Double_t*");
      //fNumber = -1;
      gROOT->GetListOfFunctions()->Add(this);
      if (! fMethodCall->IsValid() ) {
         Error("TFn","No function found with the signature %s(Double_t*,Double_t*)",funcname);
      }
   } else {
      Error("TFn","can not find any function at the address 0x%lx. This function requested for %s",(Long_t)fcn,name);
   }


}


//______________________________________________________________________________
TFn::TFn(const char *name, UInt_t ndim, Double_t (*fcn)(Double_t *, Double_t *), Double_t* min, Double_t* max, UInt_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fIntegrator()
{
   // F1 constructor using a pointer to a real function.
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


   fType       = 1;
   fMethodCall = 0;
   fCintFunc   = 0;
   fFunctor = ROOT::Math::ParamFunctor(fcn);


   fParent     = 0;
   Init(ndim, min, max, npar);
/*
   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);
*/
}

//______________________________________________________________________________
TFn::TFn(const char *name, Int_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, Int_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function"),
   fIntegrator()
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

   fType       = 1;
   fMethodCall = 0;
   fCintFunc   = 0;
   fFunctor = ROOT::Math::ParamFunctor(fcn);

   fParent     = 0;

   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);

}


//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, Int_t npar ) :
   TNamed(name, "TFn created from ROOT::Math::ParamFunctor"),
   fType      ( 1 ),
   fIntegrator   (),
   fParErrors ( 0 ),
   fParMin    ( 0 ),
   fParMax    ( 0 ),
   fParent    ( 0 ),
   fMethodCall( 0 ),
   fCintFunc  ( 0 ),
   fFunctor   ( ROOT::Math::ParamFunctor(f) )
{
   // F1 constructor using the Functor class.
   //
   //   xmin and xmax define the plotting range of the function
   //   npar is the number of free parameters used by the function
   //
   //   This constructor can be used only in compiled code
   //
   // WARNING! A function created with this constructor cannot be Cloned.

   Init(ndim, min, max, npar);
   CreateFromFunctor(name);
}


//______________________________________________________________________________
void TFn::CreateFromFunctor(const char *name)
{
   // Internal Function to Create a TFn  using a Functor.
   //
   //          Used by the template constructors

/*
   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);
*/
}

//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, void* ptr, Double_t* min, Double_t* max, Int_t npar, const char* className) : 
   TNamed(name, TString::Format("CINT class - %s", className).Data()),
   fIntegrator()
{
   // F1 constructor from an interpreted class defining the operator() or Eval().
   // This constructor emulate the syntax of the template constructor using a C++ callable object (functor)
   // which can be used only in C++ compiled mode.
   // The class name is required to get the type of class given the void pointer ptr.
   // For the method name is used the operator() (Double_t *, Double_t * ).
   // Use the other constructor taking the method name for different method names.
   //
   //  xmin and xmax specify the function plotting range
   //  npar are the number of function parameters
   //
   //  see tutorial  math.exampleFunctor.C for an example of using this constructor
   //
   //  This constructor is used only when using CINT.
   //  In compiled mode the template constructor is used and in that case className is not needed

   CreateFromCintClass(name, ndim, ptr, min, max, npar, className, 0 );
}

//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, void *ptr, void *, Double_t* min, Double_t* max, Int_t npar, const char* className, const char* methodName) : 
   TNamed(name, TString::Format("CINT class - %s", className).Data()),
   fIntegrator()
{
   // F1 constructor from an interpreter class using a specified member function.
   // This constructor emulate the syntax of the template constructor using a C++ class and a given
   // member function pointer, which can be used only in C++ compiled mode.
   // The class name is required to get the type of class given the void pointer ptr.
   // The second void * is not needed for the CINT case, but is kept for emulating the API of the
   // template constructor.
   // The method name is optional. By default is looked for operator() (Double_t *, Double_t *) or
   // Eval(Double_t *, Double_t*)
   //
   //  xmin and xmax specify the function plotting range
   //  npar are the number of function parameters.
   //
   //
   //  see tutorial  math.exampleFunctor.C for an example of using this constructor
   //
   //  This constructor is used only when using CINT.
   //  In compiled mode the template constructor is used and in that case className is not needed

   CreateFromCintClass(name, ndim, ptr, min, max, npar, className, methodName);
}

//______________________________________________________________________________
void TFn::CreateFromCintClass(const char *name, Int_t ndim, void *ptr, Double_t* min, Double_t* max, Int_t npar, const char * className, const char* methodName)
{
   // Internal function used to create from TFn from an interpreter CINT class
   // with the specified type (className) and member function name (methodName).
   //

   Init(ndim, min, max, npar);

   fType       = 3;

   fParent     = 0;
   fMethodCall = 0;

   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   //SetName(name);


   if (!ptr) return;
   fCintFunc = ptr;

   if (!className) return;

   TClass *cl = TClass::GetClass(className);

   if (cl) {
      fMethodCall = new TMethodCall();


      if (methodName)
         fMethodCall->InitWithPrototype(cl,methodName,"Double_t*,Double_t*");
      else {
         fMethodCall->InitWithPrototype(cl,"operator()","Double_t*,Double_t*");
         if (! fMethodCall->IsValid() )
            // try with Eval if operator() is not found
            fMethodCall->InitWithPrototype(cl,"Eval","Double_t*,Double_t*");
      }

      //FIXME: what is this?
      //fNumber = -1;
      gROOT->GetListOfFunctions()->Add(this);
      if (! fMethodCall->IsValid() ) {
         if (methodName)
            Error("TFn","No function found in class %s with the signature %s(Double_t*,Double_t*)",className,methodName);
         else
            Error("TFn","No function found in class %s with the signature operator() (Double_t*,Double_t*) or Eval(Double_t*,Double_t*)",className);
      }
   } else {
      Error("TFn","can not find any class with name %s at the address 0x%lx",className,(Long_t)ptr);
   }


}


//______________________________________________________________________________
TFn::~TFn()
{
   // TFn default destructor.
   // TODO: delete fIntegral
   if (fParMin)    delete [] fParMin;
   if (fParMax)    delete [] fParMax;
   if (fParErrors) delete [] fParErrors;
   delete fMethodCall;

   if (fParent) fParent->RecursiveRemove(this);
}


//______________________________________________________________________________
TFn::TFn(const TFn &rhs) : TNamed(rhs), fIntegrator() 
{
   // Copy constructor.
  
   fType = rhs.fType; 
   fParent = rhs.fParent;
   fMethodCall = rhs.fMethodCall;
   fCintFunc = rhs.fCintFunc;

   Init(rhs.fNdim, rhs.fMin, rhs.fMax, rhs.fNpar);
}

//______________________________________________________________________________
TFn& TFn::operator=(const TFn& rhs)
{
   // Assignment operator.

   if (this != &rhs) {

      fType = rhs.fType;
      fFunctor = rhs.fFunctor;
      fCintFunc = rhs.fCintFunc;
      fParent = rhs.fParent;
      fNorm = rhs.fNorm;

      delete fMethodCall;
      if (rhs.fMethodCall) fMethodCall =  new TMethodCall(*fMethodCall);

      if (fNdim != rhs.fNdim) {
         fNdim = rhs.fNdim;
         delete [] fMin; 
         delete [] fMax;
      }

      if (fNpar != rhs.fNpar) {
         fNpar = rhs.fNpar;
         delete [] fParMin; 
         delete [] fParMax; 
         delete [] fParErrors;
        
         if (fNpar > 0) { 
            fParMin = new Double_t[fNpar]; fParMin = NULL;
            fParMax = new Double_t[fNpar]; fParMax = NULL;
            fParErrors = new Double_t[fNpar]; fParErrors = NULL;
            std::copy(rhs.fParMin, rhs.fParMin + fNpar, fParMin);
            std::copy(rhs.fParMax, rhs.fParMax + fNpar, fParMax);
            std::copy(rhs.fParErrors, rhs.fParErrors + fNpar, fParErrors);
         }
      }
   }

}

//______________________________________________________________________________
Double_t TFn::Eval(Double_t* x)
{
   // Evaluate this formula.
   //   Computes the value of this n-d function on the vector x.
   //   The parameters used will be the ones in the array fParams.

   InitArgs(x);
   return DoEvalPar(x);
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
   if (icoord > UInt_t(fNdim)) {
      Error("DoDerivative", "The index of the coordonate surpasses the bounds [0,%d] of the %d-dimensional \
         function %s", fNdim - 1, fNdim, GetName());
      return 0.0;
   }
   if (x == NULL) {
      Error("DoDerivative", "Vector of coordinates, x, passed as input is not allocated");
      return 0.0;
   }

   Double_t* xm = const_cast<Double_t*>(x); 
   Double_t  x0 = x[icoord];
   
   //   The step size is chosen depending on x[icoord], except when the latter is too small, in which case
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
   // WARNING. In case of an interpreted function (fType = 2), it is the
   // user's responsibility to initialize the parameters via InitArgs
   // before calling this function.
   // InitArgs should be called at least once to specify the addresses
   // of the arguments x and params.
   // InitArgs should be called everytime these addresses change.

   TFn* mutableThis = const_cast<TFn*>(this);

   if (fType == 0) {
      assert(fFormula != NULL);
      return fFormula->EvalPar(x,params);
   }
   Double_t result = 0;
   if (fType == 1)  {
      if (!mutableThis->fFunctor.Empty()) {
         if (params) result = mutableThis->fFunctor((Double_t*)x,(Double_t*)params);
         else        result = mutableThis->fFunctor((Double_t*)x,fParams);
      } // else          result = const_cast<TFn*>(this)->GetSave(x);
      return result;
   }
   if (fType == 2) {
      if (fMethodCall) fMethodCall->Execute(result);
      // else             result = const_cast<TFn*>(this)->GetSave(x);
      return result;
   }
   if (fType == 3) {
      if (fMethodCall) fMethodCall->Execute(fCintFunc,result);
      // else             result = const_cast<TFn*>(this)->GetSave(x);
      return result;
   }
   return result;
}


//______________________________________________________________________________
void TFn::FixParameter(UInt_t ipar, Double_t value)
{
   // Fix the value of a parameter
   // The specified value will be used in a fit operation

   // TODO: see how to remove the Double_t check for TF1 (it's done in SetParameter too)
   if (ipar >= fNpar) return; 

   fParams[ipar] = value;
  // Update();

   if (value != 0) SetParLimits(ipar,value,value);
   else            SetParLimits(ipar,1,1);
}


class ReverseSignTFn : public ROOT::Math::IBaseFunctionMultiDim {
public:   
   ReverseSignTFn(TFn* func) : fFunc(func) {}
   virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const {
      return new ReverseSignTFn(fFunc);
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
   ReverseSignTFn reverseSignFunc(const_cast<TFn*>(this));
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
   ReverseSignTFn reverseSignFunc(const_cast<TFn*>(this));
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
      x = localX;
   }
   for(UInt_t i = 0; i < fNdim; ++i) {
      x[i] = (min[i] + max[i]) / 2.0;
      std::cout << "x[" << i << "]: " << x[i] << " | ";
   }
   std::cout << std::endl;

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
      return 0;
   }
   return fParErrors[ipar];
}


//______________________________________________________________________________
void TFn::GetParLimits(UInt_t ipar, Double_t &parmin, Double_t &parmax) const
{
   // Return limits for parameter ipar.

   parmin = 0;
   parmax = 0;
   if (ipar >= fNpar) return;
   if (fParMin) parmin = fParMin[ipar];
   if (fParMax) parmax = fParMax[ipar];
}

//______________________________________________________________________________
void TFn::GetRange(Double_t *min, Double_t *max) const
{
   // Return range of a n-D function.
   // NOTE: the user is responsible for creating arrays of sufficient size
   memcpy(min, fMin, fNdim * sizeof(Double_t));
   memcpy(max, fMax, fNdim * sizeof(Double_t));
}


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

//______________________________________________________________________________
Double_t TFn::GradientPar(UInt_t ipar, const Double_t *x, Double_t eps)
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

   if (fNpar == 0) return 0; 

   if(eps< 1e-10 || eps > 1) {
      Warning("Derivative","parameter esp=%g out of allowed range[1e-10,1], reset to 0.01",eps);
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
   if (func->GetParError(ipar)!=0)
      h = eps*func->GetParError(ipar);
   else
      h=eps;



   fParams[ipar] = par0 + h;     f1 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 - h;     f2 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 + h/2;   g1 = func->DoEvalPar(x,fParams);
   fParams[ipar] = par0 - h/2;   g2 = func->DoEvalPar(x,fParams);

   //compute the central differences
   h2    = 1/(2.*h);
   d0    = f1 - f2;
   d2    = 2*(g1 - g2);

   Double_t  grad = h2*(4*d2 - d0)/3.;

   // restore original value
   fParams[ipar] = par0;

   return grad;
}

//______________________________________________________________________________
void TFn::GradientPar(const Double_t *x, Double_t *grad, Double_t eps)
{
   // Compute the gradient with respect to the parameters
   // Parameters:
   // x - point, were the gradient is computed
   // grad - used to return the computed gradient, assumed to be of at least fNpar size
   // eps - if the errors of parameters have been computed, the step used in
   // numerical differentiation is eps*parameter_error.
   // if the errors have not been computed, step=eps is used
   // default value of eps = 0.01
   // Method is the same as in Derivative() function
   //
   // If a paramter is fixed, the gradient on this parameter = 0

   if(eps< 1e-10 || eps > 1) {
      Warning("Derivative","parameter esp=%g out of allowed range[1e-10,1], reset to 0.01",eps);
      eps = 0.01;
   }

   for (UInt_t ipar = 0; ipar < fNpar; ++ipar){
      grad[ipar] = GradientPar(ipar,x,eps);
   }
}

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
Double_t TFn::IntegralError(Double_t a, Double_t b, const Double_t * params, const Double_t * covmat, Double_t epsilon )
{
   // Return Error on Integral of a parameteric function between a and b 
   // due to the parameter uncertainties.
   // A pointer to a vector of parameter values and to the elements of the covariance matrix (covmat)
   // can be optionally passed.  By default (i.e. when a zero pointer is passed) the current stored 
   // parameter values are used to estimate the integral error together with the covariance matrix
   // from the last fit (retrieved from the global fitter instance) 
   //
   // IMPORTANT NOTE1: When no covariance matrix is passed and in the meantime a fit is done 
   // using another function, the routine will signal an error and it will return zero only 
   // when the number of fit parameter is different than the values stored in TFn (TFn::GetNpar() ). 
   // In the case that npar is the same, an incorrect result is returned. 
   //
   // IMPORTANT NOTE2: The user must pass a pointer to the elements of the full covariance matrix 
   // dimensioned with the right size (npar*npar), where npar is the total number of parameters (TFn::GetNpar()), 
   // including also the fixed parameters. When there are fixed parameters, the pointer returned from 
   // TVirtualFitter::GetCovarianceMatrix() cannot be used. 
   // One should use the TFitResult class, as shown in the example below.   
   // 
   // To get the matrix and values from an old fit do for example:  
   // TFitResultPtr r = histo->Fit(func, "S");
   // ..... after performing other fits on the same function do 
   // func->IntegralError(x1,x2,r->GetParams(), r->GetCovarianceMatrix()->GetMatrixArray() );

   Double_t x1[1]; 
   Double_t x2[1]; 
   x1[0] = a, x2[0] = b;
   // FIXME 
   return 0.0;
   //return ROOT::TFnHelper::IntegralError(this,1,x1,x2,params,covmat,epsilon);
}

//______________________________________________________________________________
Double_t TFn::IntegralError(Int_t n, const Double_t * a, const Double_t * b, const Double_t * params, const  Double_t * covmat, Double_t epsilon )
{
   // Return Error on Integral of a parameteric function with dimension larger tan one 
   // between a[] and b[]  due to the parameters uncertainties.
   // TFn::IntegralMultiple is used for the integral calculation
   //
   // A pointer to a vector of parameter values and to the elements of the covariance matrix (covmat) can be optionally passed.
   // By default (i.e. when a zero pointer is passed) the current stored parameter values are used to estimate the integral error 
   // together with the covariance matrix from the last fit (retrieved from the global fitter instance).
   //
   // IMPORTANT NOTE1: When no covariance matrix is passed and in the meantime a fit is done 
   // using another function, the routine will signal an error and it will return zero only 
   // when the number of fit parameter is different than the values stored in TFn (TFn::GetNpar() ). 
   // In the case that npar is the same, an incorrect result is returned. 
   //
   // IMPORTANT NOTE2: The user must pass a pointer to the elements of the full covariance matrix 
   // dimensioned with the right size (npar*npar), where npar is the total number of parameters (TFn::GetNpar()), 
   // including also the fixed parameters. When there are fixed parameters, the pointer returned from 
   // TVirtualFitter::GetCovarianceMatrix() cannot be used. 
   // One should use the TFitResult class, as shown in the example below.   
   // 
   // To get the matrix and values from an old fit do for example:  
   // TFitResultPtr r = histo->Fit(func, "S");
   // ..... after performing other fits on the same function do 
   // func->IntegralError(x1,x2,r->GetParams(), r->GetCovarianceMatrix()->GetMatrixArray() );
   // FIXME
   if (fNpar == 0) {
      Error("IntegralError", "Function has no parameters");
      return 0.0;
   }
/*
   std::vector<Double_t> oldParams(fNpar);
   std::copy(fParams, fParams + fNpar, oldParams.begin());
   SetParameters(params);

   TMatrixDSym covMatrix(fNPar);

   if (covmat == NULL) {
      TVirtualFitter* vFitter = TVirtualFitter::GetFitter();
      TBackCompFitter* bcFitter = 
*/
   return 0.0;
   //return ROOT::TFnHelper::IntegralError(this,n,a,b,params,covmat,epsilon);
}

//______________________________________________________________________________
Double_t TFn::IntegralMultiple(const Double_t *a, const Double_t *b, Double_t eps, Double_t &relErr)
{
   //  See more general prototype below.
   //  This interface kept for back compatibility

   Int_t nFuncEval, status;
   Int_t minPts = (1 << fNdim) + 2 * fNdim * (fNdim + 1) + 1; //ie 7 for n=1
   Int_t maxPts = std::max(minPts, 1000);
   Double_t result = IntegralMultiple(a, b, minPts, maxPts, eps, relErr, nFuncEval, status);
   if (status != 0) Warning("IntegralMultiple","exit status code = %d, ", status);
   return result;
}


//______________________________________________________________________________
Double_t TFn::IntegralMultiple(const Double_t* a, const Double_t* b, Int_t minPts, Int_t maxPts, Double_t eps, Double_t &relErr, Int_t &nFuncEval, Int_t &status)
{
   //  Adaptive Quadrature for Multiple Integrals over N-Dimensional
   //  Rectangular Regions
   //
   //Begin_Latex
   // I_{n} = #int_{a_{n}}^{b_{n}} #int_{a_{n-1}}^{b_{n-1}} ... #int_{a_{1}}^{b_{1}} f(x_{1}, x_{2},...,x_{n}) dx_{1}dx_{2}...dx_{n}
   //End_Latex
   //
   // Author(s): A.C. Genz, A.A. Malik
   // converted/adapted by R.Brun to C++ from Fortran CERNLIB routine RADMUL (D120)
   // The new code features many changes compared to the Fortran version.
   //
   // This function computes, to an attempted specified accuracy, the value of
   // the integral over an n-dimensional rectangular region.
   //
   // Input parameters:
   //
   //    a,b   : One-dimensional arrays of length fNdim (TFn::GetNDim()). On entry A[i],  and  B[i],
   //            contain the lower and upper limits of integration, respectively.
   //    minPts: Minimum number of function evaluations requested. Must not exceed maxPts.
   //            if minPts < 1 minPts is set to 2^fNdim + 2 * fNdim * (fNdim + 1) +1
   //    maxPts: Maximum number of function evaluations to be allowed.
   //            maxPts >= 2^fNdim + 2 * fNdim * (fNdim + 1) +1
   //            if maxPts < minPts, maxPts is set to 10*minPts
   //    eps   : Specified relative accuracy.
   //
   // Output parameters:
   //
   //    relErr    : Contains, on exit, an estimation of the relative accuracy of the result.
   //    nFuncEval : number of function evaluations performed.
   //    status  :
   //        0 Normal exit.  . At least minPts and at most maxPts calls to the function were performed.
   //        1 maxPts is too small for the specified accuracy eps.
   //          The result and relErr contain the values obtainable for the
   //          specified value of maxPts.
   //        3 n<2 or n>15
   //
   // Method:
   //
   //    An integration rule of degree seven is used together with a certain
   //    strategy of subdivision.
   //    For a more detailed description of the method see References.
   //
   // Notes:
   //
   //   1.Multi-dimensional integration is time-consuming. For each rectangular
   //     subregion, the routine requires function evaluations.
   //     Careful programming of the integrand might result in substantial saving
   //     of time.
   //   2.Numerical integration usually works best for smooth functions.
   //     Some analysis or suitable transformations of the integral prior to
   //     numerical work may contribute to numerical efficiency.
   //
   // References:
   //
   //   1.A.C. Genz and A.A. Malik, Remarks on algorithm 006:
   //     An adaptive algorithm for numerical integration over
   //     an N-dimensional rectangular region, J. Comput. Appl. Math. 6 (1980) 295-302.
   //   2.A. van Doren and L. de Ridder, An adaptive algorithm for numerical
   //     integration over an n-dimensional cube, J.Comput. Appl. Math. 2 (1976) 207-217.

   ROOT::Math::AdaptiveIntegratorMultiDim aimd(*this, eps, eps, maxPts);
   aimd.SetMinPts(minPts);
   Double_t result = aimd.Integral(a,b);
   relErr = aimd.RelError();
   nFuncEval = aimd.NEval();
   status = 0;

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
   // Dump this function with its attributes.

//   TFormula::Print(option);
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
   //
   // The specified limits will be used in a fit operation
   // when the option "B" is specified (Bounds).
   // To fix a parameter, use TFn::FixParameter

   if (ipar >= fNpar) return;
   if (!fParMin) { fParMin = new Double_t[fNpar]; for (UInt_t i = 0; i < fNpar; ++i) fParMin[i] = 0; }
   if (!fParMax) { fParMax = new Double_t[fNpar]; for (UInt_t i = 0; i < fNpar; ++i) fParMax[i] = 0; }
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

   memcpy(fMin, min, fNdim * sizeof(Double_t));
   memcpy(fMax, max, fNdim * sizeof(Double_t)); 
}


//______________________________________________________________________________
void TFn::Streamer(TBuffer &b)
{
   // Stream a class object.
// TODO: rethink this
/*
   if (b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t v = b.ReadVersion(&R__s, &R__c);
      if (v > 4) {
         b.ReadClassBuffer(TFn::Class(), this, v, R__s, R__c);
         if (v == 5 && fNsave > 0) {
            //correct badly saved fSave in 3.00/06
            Int_t np = fNsave - 3;
            fSave[np]   = fSave[np-1];
            fSave[np+1] = fXmin;
            fSave[np+2] = fXmax;
         }
         return;
      }
      //====process old versions before automatic schema evolution
      TFormula::Streamer(b);
      TAttLine::Streamer(b);
      TAttFill::Streamer(b);
      TAttMarker::Streamer(b);
      if (v < 4) {
         Float_t xmin,xmax;
         b >> xmin; fXmin = xmin;
         b >> xmax; fXmax = xmax;
      } else {
         b >> fXmin;
         b >> fXmax;
      }
      b >> fNpx;
      b >> fType;
      b.ReadArray(fParErrors);
      if (v > 1) {
         b.ReadArray(fParMin);
         b.ReadArray(fParMax);
      } else {
         fParMin = new Double_t[fNpar+1];
         fParMax = new Double_t[fNpar+1];
      }
      if (v == 1) {
      }
      if (v > 1) {
         if (v < 4) { // XXX fMaximum was here
         } else {
         }
      }
      if (v > 2) {
         b >> fNsave;
         if (fNsave > 0) {
            fSave = new Double_t[fNsave+10];
            b.ReadArray(fSave);
            //correct fSave limits to match new version
            fSave[fNsave]   = fSave[fNsave-1];
            fSave[fNsave+1] = fSave[fNsave+2];
            fSave[fNsave+2] = fSave[fNsave+3];
            fNsave += 3;
         } else fSave = 0;
      }
      b.CheckByteCount(R__s, R__c, TFn::IsA());
      //====end of old versions

   } else {
      Int_t saved = 0;
      if (fType > 0 && fNsave <= 0) { saved = 1; Save(fXmin,fXmax,0,0,0,0);}

      b.WriteClassBuffer(TFn::Class(),this);

      if (saved) {delete [] fSave; fSave = 0; fNsave = 0;}
   }
*/
}

/**
 * TFn_Projection1D is a helper / wrapper class used to fix one coordinate of the original
 * TFn and allow integration only on the other n-1 coordinates. 
 */
// TODO: maybe derive from TFn
class TFn_Projection1D : ROOT::Math::IBaseFunctionMultiDim {
public:
   // assumes correct arguments are passed
   // TODO: constify !!!
   TFn_Projection1D(TFn& func, UInt_t idxCoord) :
      fFunc(func), fNdim(fFunc.NDim()), fIdxCoord(idxCoord), fValCoord(0.0), fIntegral()
   {
      fX = new Double_t[fNdim];
      fMin = new Double_t[fNdim];
      fMax = new Double_t[fNdim];
      fIntegral.SetFunction(*this);

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
      fValCoord = *valCoord;
      return fIntegral.Integral(fMin, fMax);
   }
      
protected:
   // NOTE: DO NOT USE !!! Shallow copying
   virtual TFn_Projection1D* Clone() const { return new TFn_Projection1D(fFunc, fIdxCoord); } 
   
private:
   virtual Double_t DoEval(const Double_t *x) const {
      assert(x != NULL);
      for(UInt_t i = 0; i < fIdxCoord; ++i) fX[i] = x[i];
      fX[fIdxCoord] = fValCoord;
      for(UInt_t i = fIdxCoord + 1; i < fNdim; ++i) fX[i] = x[i - 1]; 
      return fFunc(fX);
   }
     
   TFn& fFunc;
   UInt_t fNdim; // dimension of TFn
   UInt_t fIdxCoord;
   mutable Double_t fValCoord;
   ROOT::Math::AdaptiveIntegratorMultiDim fIntegral;
   
   Double_t* fX;   //![fNdim] contains (n-1)-dim input array plus fixed value on specified coord
   Double_t* fMin; //![fNdim] contains original TFn min range
   Double_t* fMax; //![fNdim] contains original TFn max range
};

//______________________________________________________________________________
TF1* TFn::Projection1D(UInt_t idxCoord) const
{
   // Return the 1D projection of the function on the icoord coordonate
   // 
   // NOTE: The TFn and its projection are intrinsically linked; if one changes
   // (ranges for example), the other will as well

   // FIXME: return this
   if(fNdim == 1) return NULL;

   if(idxCoord >= fNdim) {
      Error("Projection1D", "Coordonate index passed as input is out of dimensional range");
      return NULL;
   }

   // FIXME: remove const hack
   TFn_Projection1D* proj =  new TFn_Projection1D(*(TFn*)this, idxCoord);

   return new TF1(TString::Format("%s_Projection1D", GetName()), proj, &TFn_Projection1D::Integral,
      fMin[idxCoord], fMax[idxCoord], fNpar, "TFn_Projection1D", "Integral");
   return new TF1();  
}



