// @(#)root/hist:$Id$
// Author: Rene Brun   18/08/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

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

Bool_t TFn::fgAbsValue    = kFALSE;
Bool_t TFn::fgRejectPoint = kFALSE;
static Double_t gErrorTFn = 0;


ClassImp(TFn)

// class wrapping evaluation of TFn(x) - y0
class GFunc {
   const TFn* fFunction;
   const double fY0;
public:
   GFunc(const TFn* function , double y ):fFunction(function), fY0(y) {}
   double operator()(double x) const {
      // FIXME: return fFunction->Eval(x) - fY0;
      return 0.0;
   }
};

// class wrapping evaluation of -TFn(x)
class GInverseFunc {
   const TFn* fFunction;
public:
   GInverseFunc(const TFn* function):fFunction(function) {}
   double operator()(double x) const {
// FIXME:      return - fFunction->Eval(x);
      return 0.0; 
   }
};

// class wrapping function evaluation directly in n-Dim interface (used for integration) 
// and implementing the methods for the momentum calculations

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
/*
   ROOT::Math::IGenFunction * Clone()  const { 
      // use default copy constructor
      TFn_EvalWrapper * f =  new TFn_EvalWrapper( *this);
      f->fFunc->InitArgs(f->fX, f->fPar); 
      return f;
   }*/
   // evaluate |f(x)|
   Double_t DoEval( Double_t* x) const {  
      Double_t fval = fFunc->EvalPar( x, fPar);
      return (fAbsVal && fval < 0) ? -fval : fval;
   } 
   // evaluate x * |f(x)|
   Double_t EvalFirstMom( Double_t x) { 
      fX[0] = x; 
      return fX[0] * TMath::Abs( fFunc->EvalPar( fX, fPar) ); 
   } 
   // evaluate (x - x0) ^n * f(x)
   Double_t EvalNMom( Double_t x) const  { 
      fX[0] = x; 
      return TMath::Power( fX[0] - fX0, fN) * TMath::Abs( fFunc->EvalPar( fX, fPar) ); 
   }

   TFn * fFunc; 
   mutable Double_t fX[1]; 
   const double * fPar; 
   Bool_t fAbsVal;
   Double_t fN; 
   Double_t fX0;
};

class Test: public ROOT::Math::IGenFunction { };

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
   TFn *fa1 = new TFn("fa1","sin(x)/x",0,10);
   fa1->Draw();
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
A TFn can be created from any C++ class implementing the operator()(double *x, double *p).
The advantage of the function object is that he can have a state and reference therefore what-ever other object.
In this way the user can customize his function.
<p>Example:
<div class="code"><pre>
class  MyFunctionObject {
 public:
   // use constructor to customize your function object

   double operator() (double *x, double *p) {
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
(double * , double *) and returning a double.
<p>Example:
<div class="code"><pre>
class  MyFunction {
 public:
   ...
   double Evaluate() (double *x, double *p) {
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
void TFn::Init(Int_t ndim, Double_t* min, Double_t* max) 
{
   // TFn initializer, employed by constructors
   if (ndim > 0) {
      fNdim = ndim; // XXX: should we put this in the initialization list?
      fMin = new Double_t[fNdim];
      fMax = new Double_t[fNdim];
      memcpy(fMin, min, fNdim * sizeof(Double_t));
      memcpy(fMax, max, fNdim * sizeof(Double_t));
   }
} 

//______________________________________________________________________________
TFn::TFn() : TNamed()
{
   // F1 default constructor.

   fMin       = NULL;
   fMax       = NULL;
   fNpx       = 100;
   fType      = 0;
   fNpfits    = 0;
   fNDF       = 0;
   fNsave     = 0;
   fIntegral  = 0;
   fParErrors = 0;
   fParMin    = 0;
   fParMax    = 0;
   fAlpha     = 0;
   fBeta      = 0;
   fGamma     = 0;
   fParent    = 0;
   fSave      = 0;
   fHistogram = 0;
   fMinimum   = -1111;
   fMaximum   = -1111;
   fMethodCall = 0;
   fCintFunc   = 0;
}

//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, void* fcn, Double_t* min, Double_t* max, Int_t npar) :
   TNamed(name, "TFn created from a pointer to an interpreted function")
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

   Init(ndim, min, max);

   fNpx        = 100;
   fType       = 2;
   //fFunction   = 0;
   if (npar > 0) {
      fNpar = npar;
      //fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      for (int i = 0; i < fNpar; i++) {
         fParams[i]     = 0;
         fParErrors[i]  = 0;
         fParMin[i]     = 0;
         fParMax[i]     = 0;
      }
   } else {
      fParErrors = 0;
      fParMin    = 0;
      fParMax    = 0;
   }
   fIntegral   = 0;
   fAlpha      = 0;
   fBeta       = 0;
   fGamma      = 0;
   fParent     = 0;
   fNpfits     = 0;
   fNDF        = 0;
   fNsave      = 0;
   fSave       = 0;
   fHistogram  = 0;
   fMinimum    = -1111;
   fMaximum    = -1111;
   fMethodCall = 0;
   fCintFunc   = 0;
   fNdim       = 1;

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
TFn::TFn(const char *name, Int_t ndim, Double_t (*fcn)(Double_t *, Double_t *), Double_t* min, Double_t* max, Int_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function")
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

   Init(ndim, min, max);

   fNpx        = 100;

   fType       = 1;
   fMethodCall = 0;
   fCintFunc   = 0;
   fFunctor = ROOT::Math::ParamFunctor(fcn);
    
   if (npar > 0) {
      fNpar = npar;
      //fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      for (int i = 0; i < fNpar; i++) {
         fParams[i]     = 0;
         fParErrors[i]  = 0;
         fParMin[i]     = 0;
         fParMax[i]     = 0;
      }
   } else {
      fParErrors = 0;
      fParMin    = 0;
      fParMax    = 0;
   }
   fIntegral   = 0;
   fAlpha      = 0;
   fBeta       = 0;
   fGamma      = 0;
   fNsave      = 0;
   fSave       = 0;
   fParent     = 0;
   fNpfits     = 0;
   fNDF        = 0;
   fHistogram  = 0;
   fMinimum    = -1111;
   fMaximum    = -1111;
   fNdim       = 1;

   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);

}

//______________________________________________________________________________
TFn::TFn(const char *name, Int_t ndim, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t* min, Double_t* max, Int_t npar) : 
   TNamed(name, "TFn created from a pointer to a real function")
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

   Init(ndim, min, max);

   fNpx        = 100;

   fType       = 1;
   fMethodCall = 0;
   fCintFunc   = 0;
   fFunctor = ROOT::Math::ParamFunctor(fcn);

   if (npar > 0) {
      fNpar = npar;
      //fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      for (int i = 0; i < fNpar; i++) {
         fParams[i]     = 0;
         fParErrors[i]  = 0;
         fParMin[i]     = 0;
         fParMax[i]     = 0;
      }
   } else {
      fParErrors = 0;
      fParMin    = 0;
      fParMax    = 0;
   }
   fIntegral   = 0;
   fAlpha      = 0;
   fBeta       = 0;
   fGamma      = 0;
   fNsave      = 0;
   fSave       = 0;
   fParent     = 0;
   fNpfits     = 0;
   fNDF        = 0;
   fHistogram  = 0;
   fMinimum    = -1111;
   fMaximum    = -1111;
   fNdim       = 1;

   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);

}


//______________________________________________________________________________
TFn::TFn(const char*name, Int_t ndim, ROOT::Math::ParamFunctor f, Double_t* min, Double_t* max, Int_t npar ) :
   TNamed(name, "TFn created from ROOT::Math::ParamFunctor"),
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

   Init(ndim, min, max);
   CreateFromFunctor(name, npar);
}


//______________________________________________________________________________
void TFn::CreateFromFunctor(const char *name, Int_t npar)
{
   // Internal Function to Create a TFn  using a Functor.
   //
   //          Used by the template constructors

   fNdim       = 1;

   if (npar > 0) {
      fNpar = npar;
      //fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      for (int i = 0; i < fNpar; i++) {
         fParams[i]     = 0;
         fParErrors[i]  = 0;
         fParMin[i]     = 0;
         fParMax[i]     = 0;
      }
   } else {
      fParErrors = 0;
      fParMin    = 0;
      fParMax    = 0;
   }

   // Store formula in linked list of formula in ROOT
   TFn *f1old = (TFn*)gROOT->GetListOfFunctions()->FindObject(name);
   gROOT->GetListOfFunctions()->Remove(f1old);
   SetName(name);
   gROOT->GetListOfFunctions()->Add(this);

}

//______________________________________________________________________________
TFn::TFn(const char* name, Int_t ndim, void* ptr, Double_t* min, Double_t* max, Int_t npar, const char* className) : 
   TNamed(name, TString::Format("CINT class - %s", className).Data())
{
   // F1 constructor from an interpreted class defining the operator() or Eval().
   // This constructor emulate the syntax of the template constructor using a C++ callable object (functor)
   // which can be used only in C++ compiled mode.
   // The class name is required to get the type of class given the void pointer ptr.
   // For the method name is used the operator() (double *, double * ).
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
   TNamed(name, TString::Format("CINT class - %s", className).Data())
{
   // F1 constructor from an interpreter class using a specified member function.
   // This constructor emulate the syntax of the template constructor using a C++ class and a given
   // member function pointer, which can be used only in C++ compiled mode.
   // The class name is required to get the type of class given the void pointer ptr.
   // The second void * is not needed for the CINT case, but is kept for emulating the API of the
   // template constructor.
   // The method name is optional. By default is looked for operator() (double *, double *) or
   // Eval(double *, double*)
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

   Init(ndim, min, max);

   fNpx        = 100;
   fType       = 3;
   if (npar > 0) {
      fNpar = npar;
     // fNames      = new TString[fNpar];
      fParams     = new Double_t[fNpar];
      fParErrors  = new Double_t[fNpar];
      fParMin     = new Double_t[fNpar];
      fParMax     = new Double_t[fNpar];
      for (int i = 0; i < fNpar; i++) {
         fParams[i]     = 0;
         fParErrors[i]  = 0;
         fParMin[i]     = 0;
         fParMax[i]     = 0;
      }
   } else {
      fParErrors = 0;
      fParMin    = 0;
      fParMax    = 0;
   }
   fIntegral   = 0;
   fAlpha      = 0;
   fBeta       = 0;
   fGamma      = 0;
   fParent     = 0;
   fNpfits     = 0;
   fNDF        = 0;
   fNsave      = 0;
   fSave       = 0;
   fHistogram  = 0;
   fMinimum    = -1111;
   fMaximum    = -1111;
   fMethodCall = 0;
   fNdim       = 1;

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
TFn& TFn::operator=(const TFn &rhs)
{
   // Operator =

   if (this != &rhs) {
      rhs.Copy(*this);
   }
   return *this;
}


//______________________________________________________________________________
TFn::~TFn()
{
   // TFn default destructor.

   if (fParMin)    delete [] fParMin;
   if (fParMax)    delete [] fParMax;
   if (fParErrors) delete [] fParErrors;
   if (fIntegral)  delete [] fIntegral;
   if (fAlpha)     delete [] fAlpha;
   if (fBeta)      delete [] fBeta;
   if (fGamma)     delete [] fGamma;
   if (fSave)      delete [] fSave;
   delete fHistogram;
   delete fMethodCall;

   if (fParent) fParent->RecursiveRemove(this);
}


//______________________________________________________________________________
TFn::TFn(const TFn &f1) : TNamed(f1) 
{
   // Constuctor.

   fMin       = NULL;
   fMax       = NULL;
   fNpx       = 100;
   fType      = 0;
   fNpfits    = 0;
   fNDF       = 0;
   fNsave     = 0;
   fIntegral  = 0;
   fParErrors = 0;
   fParMin    = 0;
   fParMax    = 0;
   fAlpha     = 0;
   fBeta      = 0;
   fGamma     = 0;
   fParent    = 0;
   fSave      = 0;
   fHistogram = 0;
   fMinimum   = -1111;
   fMaximum   = -1111;
   fMethodCall = 0;
   fCintFunc   = 0;

   ((TFn&)f1).Copy(*this);
}


//______________________________________________________________________________
void TFn::AbsValue(Bool_t flag)
{
   // Static function: set the fgAbsValue flag.
   // By default TFn::Integral uses the original function value to compute the integral
   // However, TFn::Moment, CentralMoment require to compute the integral
   // using the absolute value of the function.

   fgAbsValue = flag;
}


//______________________________________________________________________________
void TFn::Copy(TObject &obj) const
{
   // Copy this F1 to a new F1.
   // Note that the cached integral with its related arrays are not copied
   // (they are also set as transient data members) 


   TFn& rhs = ((TFn&)obj);

   if (((TFn&)obj).fParMin)    delete [] ((TFn&)obj).fParMin;
   if (((TFn&)obj).fParMax)    delete [] ((TFn&)obj).fParMax;
   if (((TFn&)obj).fParErrors) delete [] ((TFn&)obj).fParErrors;
   if (((TFn&)obj).fIntegral)  delete [] ((TFn&)obj).fIntegral;
   if (((TFn&)obj).fAlpha)     delete [] ((TFn&)obj).fAlpha;
   if (((TFn&)obj).fBeta)      delete [] ((TFn&)obj).fBeta;
   if (((TFn&)obj).fGamma)     delete [] ((TFn&)obj).fGamma;
   if (((TFn&)obj).fSave)      delete [] ((TFn&)obj).fSave;
   delete ((TFn&)obj).fHistogram;
   delete ((TFn&)obj).fMethodCall;


   if(rhs.fNdim != fNdim) {
      delete [] rhs.fMin;
      delete [] rhs.fMax;
      rhs.fMin = new Double_t[fNdim];
      rhs.fMax = new Double_t[fNdim];
   }
   memcpy(rhs.fMin, fMin, fNdim * sizeof(Double_t));
   memcpy(rhs.fMax, fMax, fNdim * sizeof(Double_t));


   ((TFn&)obj).fNpx  = fNpx;
   ((TFn&)obj).fType = fType;
   ((TFn&)obj).fCintFunc  = fCintFunc;
   ((TFn&)obj).fFunctor   = fFunctor;
   ((TFn&)obj).fNpfits  = fNpfits;
   ((TFn&)obj).fNDF     = fNDF;
   ((TFn&)obj).fMinimum = fMinimum;
   ((TFn&)obj).fMaximum = fMaximum;

   ((TFn&)obj).fParErrors = 0;
   ((TFn&)obj).fParMin    = 0;
   ((TFn&)obj).fParMax    = 0;
   ((TFn&)obj).fIntegral  = 0;
   ((TFn&)obj).fAlpha     = 0;
   ((TFn&)obj).fBeta      = 0;
   ((TFn&)obj).fGamma     = 0;
   ((TFn&)obj).fParent    = fParent;
   ((TFn&)obj).fNsave     = fNsave;
   ((TFn&)obj).fSave      = 0;
   ((TFn&)obj).fHistogram = 0;
   ((TFn&)obj).fMethodCall = 0;
   if (fNsave) {
      ((TFn&)obj).fSave = new Double_t[fNsave];
      for (Int_t j=0;j<fNsave;j++) ((TFn&)obj).fSave[j] = fSave[j];
   }
   if (fNpar) {
      ((TFn&)obj).fParErrors = new Double_t[fNpar];
      ((TFn&)obj).fParMin    = new Double_t[fNpar];
      ((TFn&)obj).fParMax    = new Double_t[fNpar];
      Int_t i;
      for (i=0;i<fNpar;i++)   ((TFn&)obj).fParErrors[i] = fParErrors[i];
      for (i=0;i<fNpar;i++)   ((TFn&)obj).fParMin[i]    = fParMin[i];
      for (i=0;i<fNpar;i++)   ((TFn&)obj).fParMax[i]    = fParMax[i];
   }
   if (fMethodCall) {
      // use copy-constructor of TMethodCall 
      TMethodCall *m = new TMethodCall(*fMethodCall);
//       m->InitWithPrototype(fMethodCall->GetMethodName(),fMethodCall->GetProto());
      ((TFn&)obj).fMethodCall  = m;
   }
}

//______________________________________________________________________________
Double_t TFn::Eval(Double_t* x)
{
   // Evaluate this formula.
   //   Computes the value of this n-d function on the vector x.
   //   The parameters used will be the ones in the array fParams.

   InitArgs(x);
   return EvalPar(x);
}


//______________________________________________________________________________
Double_t TFn::EvalPar(const Double_t *x, const Double_t *params)
{
   // Evaluate function with given coordinates and parameters.
   //
   // Compute the value of this function at point defined by array x
   // and current values of parameters in array params.
   // If argument params is omitted or equal 0, the internal values
   // of parameters (array fParams) will be used instead.
   // For a 1-D function only x[0] must be given.
   // In case of a multi-dimemsional function, the arrays x must be
   // filled with the corresponding number of dimensions.
   //
   // WARNING. In case of an interpreted function (fType=2), it is the
   // user's responsability to initialize the parameters via InitArgs
   // before calling this function.
   // InitArgs should be called at least once to specify the addresses
   // of the arguments x and params.
   // InitArgs should be called everytime these addresses change.


   if (fType == 0) {
      // TODO: include TFormula in TFn and call EvalPar
      //return TFormula::EvalPar(x,params);
      return 0.0;
   }
   Double_t result = 0;
   if (fType == 1)  {
//       if (fFunction) {
//          if (params) result = (*fFunction)((Double_t*)x,(Double_t*)params);
//          else        result = (*fFunction)((Double_t*)x,fParams);
      if (!fFunctor.Empty()) {
         if (params) result = fFunctor((Double_t*)x,(Double_t*)params);
         else        result = fFunctor((Double_t*)x,fParams);

      }else          result = GetSave(x);
      return result;
   }
   if (fType == 2) {
      if (fMethodCall) fMethodCall->Execute(result);
      else             result = GetSave(x);
      return result;
   }
   if (fType == 3) {
      //std::cout << "Eval interp function object  " << fCintFunc << " result = " << result << std::endl;
      if (fMethodCall) fMethodCall->Execute(fCintFunc,result);
      else             result = GetSave(x);
      return result;
   }
   return result;
}


//______________________________________________________________________________
void TFn::FixParameter(Int_t ipar, Double_t value)
{
   // Fix the value of a parameter
   // The specified value will be used in a fit operation

   // TODO: see how to remove the double check for TF1 (it's done in SetParameter too)
   if (ipar < 0 || ipar >= fNpar) return; 
   fParams[ipar] = value;
   Update();

   if (value != 0) SetParLimits(ipar,value,value);
   else            SetParLimits(ipar,1,1);
}


//______________________________________________________________________________
Double_t TFn::GetMaximum(Double_t* min, Double_t* max, Double_t epsilon, Int_t maxiter, Bool_t logx) const
{
   // Return the maximum value of the function
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx.
   //  This way, the step size can be controlled via the SetNpx() function.
   //  If the function is unimodal or if its extrema are far apart, setting
   //  the fNpx to a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval
   //  epsilon (default = 1.E-10) controls the relative accuracy (if |x| > 1 ) 
   //  and absolute (if |x| < 1)  and maxiter (default = 100) controls the maximum number 
   //  of iteration of the Brent algorithm
   //  If the flag logx is set the grid search is done in log step size
   //  This is done automatically if the log scale is set in the current Pad
   //
   // NOTE: see also TFn::GetMaximumX and TFn::GetX

   Double_t* usedMin; // if the input is not suitable, we bounce back to the object range minimum
   Double_t* usedMax; // if the input is not suitable, we bounce back to the object range maximum

   if (min == NULL) usedMin = fMin;
   else usedMin = min;

   if (max == NULL) usedMax = fMax;
   else usedMax = max;

   // fix invalid ranges
   for(Int_t i = 0; i < fNdim; ++i) {
      if(min[i] >= max[i]) {
         min[i] = fMin[i];
         max[i] = fMax[i];
      }
   }

   // Create default minimizer
   const char* minimizerName = ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str();
   const char* minimizerAlgo = ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str();
   ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer(minimizerName, minimizerAlgo);
   R__ASSERT(minimizer != NULL);

   if(epsilon > 0) minimizer->SetTolerance(epsilon);
   if(maxiter > 0) minimizer->SetMaxFunctionCalls(maxiter);
/*
   // need wrapper 
   GInverseFuncNdim invfunc(this);
   ROOT::Math::WrappedFunction<GInverseFunc> wf1(g);
   bm.SetFunction( wf1, xmin, xmax );
   bm.SetNpx(fNpx);
   bm.SetLogScan(logx);
   bm.Minimize(maxiter, epsilon, epsilon );
   Double_t x;
   x = - bm.FValMinimum();

   return x;*/ return 0.0;
}


//______________________________________________________________________________
Double_t TFn::GetMaximumX(Double_t xmin, Double_t xmax, Double_t epsilon, Int_t maxiter,Bool_t logx) const
{
   // Return the X value corresponding to the maximum value of the function
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx.
   //  This way, the step size can be controlled via the SetNpx() function.
   //  If the function is unimodal or if its extrema are far apart, setting
   //  the fNpx to a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval
   //  epsilon (default = 1.E-10) controls the relative accuracy (if |x| > 1 ) 
   //  and absolute (if |x| < 1)  and maxiter (default = 100) controls the maximum number 
   //  of iteration of the Brent algorithm
   //  If the flag logx is set the grid search is done in log step size
   //  This is done automatically if the log scale is set in the current Pad
    //
   // NOTE: see also TFn::GetX
   // TODO: rethink this
/* 
   if (xmin >= xmax) {xmin = fXmin; xmax = fXmax;}

   if (!logx && gPad != 0) logx = gPad->GetLogx(); 

   ROOT::Math::BrentMinimizer1D bm;
   GInverseFunc g(this);
   ROOT::Math::WrappedFunction<GInverseFunc> wf1(g);
   bm.SetFunction( wf1, xmin, xmax );
   bm.SetNpx(fNpx);
   bm.SetLogScan(logx);
   bm.Minimize(maxiter, epsilon, epsilon );
   Double_t x;
   x = bm.XMinimum();

   return x;*/ return 0.0;
}


//______________________________________________________________________________
Double_t TFn::GetMinimum(Double_t xmin, Double_t xmax, Double_t epsilon, Int_t maxiter, Bool_t logx) const
{
   // Returns the minimum value of the function on the (xmin, xmax) interval
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx. This way, the step size
   //  can be controlled via the SetNpx() function. If the function is
   //  unimodal or if its extrema are far apart, setting the fNpx to
   //  a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval
   //  epsilon (default = 1.E-10) controls the relative accuracy (if |x| > 1 ) 
   //  and absolute (if |x| < 1)  and maxiter (default = 100) controls the maximum number 
   //  of iteration of the Brent algorithm
   //  If the flag logx is set the grid search is done in log step size
   //  This is done automatically if the log scale is set in the current Pad
   //
   // NOTE: see also TFn::GetMaximumX and TFn::GetX
/*
   if (xmin >= xmax) {xmin = fXmin; xmax = fXmax;}

   if (!logx && gPad != 0) logx = gPad->GetLogx(); 

   ROOT::Math::BrentMinimizer1D bm;
   ROOT::Math::WrappedFunction<const TFn&> wf1(*this);
   bm.SetFunction( wf1, xmin, xmax );
   bm.SetNpx(fNpx);
   bm.SetLogScan(logx);
   bm.Minimize(maxiter, epsilon, epsilon );
   Double_t x;
   x = bm.FValMinimum();

   return x;
   */ return 0.0;
}


//______________________________________________________________________________
Double_t TFn::GetMinimumX(Double_t xmin, Double_t xmax, Double_t epsilon, Int_t maxiter, Bool_t logx) const
{
   // Returns the X value corresponding to the minimum value of the function
   // on the (xmin, xmax) interval
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx. This way, the step size
   //  can be controlled via the SetNpx() function. If the function is
   //  unimodal or if its extrema are far apart, setting the fNpx to
   //  a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval
   //  epsilon (default = 1.E-10) controls the relative accuracy (if |x| > 1 ) 
   //  and absolute (if |x| < 1)  and maxiter (default = 100) controls the maximum number 
   //  of iteration of the Brent algorithm
   //  If the flag logx is set the grid search is done in log step size
   //  This is done automatically if the log scale is set in the current Pad
   //
   // NOTE: see also TFn::GetX
/*
   if (xmin >= xmax) {xmin = fXmin; xmax = fXmax;}

   ROOT::Math::BrentMinimizer1D bm;
   ROOT::Math::WrappedFunction<const TFn&> wf1(*this);
   bm.SetFunction( wf1, xmin, xmax );
   bm.SetNpx(fNpx);
   bm.SetLogScan(logx);
   bm.Minimize(maxiter, epsilon, epsilon );
   Double_t x;
   x = bm.XMinimum();

   return x;*/ return 0.0;
}

/*
//______________________________________________________________________________
Double_t TFn::GetX(Double_t fy, Double_t xmin, Double_t xmax, Double_t epsilon, Int_t maxiter, Bool_t logx) const
{
   // Returns the X value corresponding to the function value fy for (xmin<x<xmax).
   // in other words it can find the roots of the function when fy=0 and successive calls
   // by changing the next call to [xmin+eps,xmax] where xmin is the previous root.
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx. This way, the step size
   //  can be controlled via the SetNpx() function. If the function is
   //  unimodal or if its extrema are far apart, setting the fNpx to
   //  a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval
   //  epsilon (default = 1.E-10) controls the relative accuracy (if |x| > 1 ) 
   //  and absolute (if |x| < 1)  and maxiter (default = 100) controls the maximum number 
   //  of iteration of the Brent algorithm
   //  If the flag logx is set the grid search is done in log step size
   //  This is done automatically if the log scale is set in the current Pad
   //
   // NOTE: see also TFn::GetMaximumX, TFn::GetMinimumX

   if (xmin >= xmax) {xmin = fXmin; xmax = fXmax;}

   if (!logx && gPad != 0) logx = gPad->GetLogx(); 

   GFunc g(this, fy);
   ROOT::Math::WrappedFunction<GFunc> wf1(g);
   ROOT::Math::BrentRootFinder brf;
   brf.SetFunction(wf1,xmin,xmax);
   brf.SetNpx(fNpx);
   brf.SetLogScan(logx);
   brf.Solve(maxiter, epsilon, epsilon);
   return brf.Root();

}
*/

//______________________________________________________________________________
Int_t TFn::GetNDF() const
{
   // Return the number of degrees of freedom in the fit
   // the fNDF parameter has been previously computed during a fit.
   // The number of degrees of freedom corresponds to the number of points
   // used in the fit minus the number of free parameters.

   if (fNDF == 0 && (fNpfits > fNpar) ) return fNpfits-fNpar;
   return fNDF;
}


//______________________________________________________________________________
Int_t TFn::GetNumberFreeParameters() const
{
   // Return the number of free parameters

   Int_t nfree = fNpar;
   Double_t al,bl;
   for (Int_t i=0;i<fNpar;i++) {
      ((TFn*)this)->GetParLimits(i,al,bl);
      if (al*bl != 0 && al >= bl) nfree--;
   }
   return nfree;
}


//______________________________________________________________________________
Double_t TFn::GetParError(Int_t ipar) const
{
   // Return value of parameter number ipar

   if (ipar < 0 || ipar > fNpar-1) return 0;
   return fParErrors[ipar];
}


//______________________________________________________________________________
void TFn::GetParLimits(Int_t ipar, Double_t &parmin, Double_t &parmax) const
{
   // Return limits for parameter ipar.

   parmin = 0;
   parmax = 0;
   if (ipar < 0 || ipar > fNpar-1) return;
   if (fParMin) parmin = fParMin[ipar];
   if (fParMax) parmax = fParMax[ipar];
}

//______________________________________________________________________________
Double_t TFn::GetRandom()
{
   // Return a random number following this function shape
   //
   //   The distribution contained in the function fname (TFn) is integrated
   //   over the channel contents.
   //   It is normalized to 1.
   //   For each bin the integral is approximated by a parabola.
   //   The parabola coefficients are stored as non persistent data members
   //   Getting one random number implies:
   //     - Generating a random number between 0 and 1 (say r1)
   //     - Look in which bin in the normalized integral r1 corresponds to
   //     - Evaluate the parabolic curve in the selected bin to find
   //       the corresponding X value.
   //   if the ratio fXmax/fXmin > fNpx the integral is tabulated in log scale in x
   //   The parabolic approximation is very good as soon as the number
   //   of bins is greater than 50.

   //  Check if integral array must be build
/*   if (fIntegral == 0) {
      fIntegral = new Double_t[fNpx+1];
      fAlpha    = new Double_t[fNpx+1];
      fBeta     = new Double_t[fNpx];
      fGamma    = new Double_t[fNpx];
      fIntegral[0] = 0;
      fAlpha[fNpx] = 0;
      Double_t integ;
      Int_t intNegative = 0;
      Int_t i;
      Bool_t logbin = kFALSE;
      Double_t dx;
      Double_t xmin = fXmin;
      Double_t xmax = fXmax;
      if (xmin > 0 && xmax/xmin> fNpx) {
         logbin =  kTRUE;
         fAlpha[fNpx] = 1;
         xmin = TMath::Log10(fXmin);
         xmax = TMath::Log10(fXmax);
      }
      dx = (xmax-xmin)/fNpx;
         
      Double_t *xx = new Double_t[fNpx+1];
      for (i=0;i<fNpx;i++) {
            xx[i] = xmin +i*dx;
      }
      xx[fNpx] = xmax;
      for (i=0;i<fNpx;i++) {
         if (logbin) {
            integ = Integral(TMath::Power(10,xx[i]), TMath::Power(10,xx[i+1]));
         } else {
            integ = Integral(xx[i],xx[i+1]);
         }
         if (integ < 0) {intNegative++; integ = -integ;}
         fIntegral[i+1] = fIntegral[i] + integ;
      }
      if (intNegative > 0) {
         Warning("GetRandom","function:%s has %d negative values: abs assumed",GetName(),intNegative);
      }
      if (fIntegral[fNpx] == 0) {
         delete [] xx;
         Error("GetRandom","Integral of function is zero");
         return 0;
      }
      Double_t total = fIntegral[fNpx];
      for (i=1;i<=fNpx;i++) {  // normalize integral to 1
         fIntegral[i] /= total;
      }
      //the integral r for each bin is approximated by a parabola
      //  x = alpha + beta*r +gamma*r**2
      // compute the coefficients alpha, beta, gamma for each bin
      Double_t x0,r1,r2,r3;
      for (i=0;i<fNpx;i++) {
         x0 = xx[i];
         r2 = fIntegral[i+1] - fIntegral[i];
         if (logbin) r1 = Integral(TMath::Power(10,x0),TMath::Power(10,x0+0.5*dx))/total;
         else        r1 = Integral(x0,x0+0.5*dx)/total;
         r3 = 2*r2 - 4*r1;
         if (TMath::Abs(r3) > 1e-8) fGamma[i] = r3/(dx*dx);
         else           fGamma[i] = 0;
         fBeta[i]  = r2/dx - fGamma[i]*dx;
         fAlpha[i] = x0;
         fGamma[i] *= 2;
      }
      delete [] xx;
   }

   // return random number
   Double_t r  = gRandom->Rndm();
   Int_t bin  = TMath::BinarySearch(fNpx,fIntegral,r);
   Double_t rr = r - fIntegral[bin];

   Double_t yy;
   if(fGamma[bin] != 0)
      yy = (-fBeta[bin] + TMath::Sqrt(fBeta[bin]*fBeta[bin]+2*fGamma[bin]*rr))/fGamma[bin];
   else
      yy = rr/fBeta[bin];
   Double_t x = fAlpha[bin] + yy;
   if (fAlpha[fNpx] > 0) return TMath::Power(10,x);
   return x;*/
   return 0.0;
}


//______________________________________________________________________________
void TFn::GetRange(Double_t *min, Double_t *max) const
{
   // Return range of a n-D function.
   // NOTE: the user is responsible for deleting the arrays

   delete [] min; delete [] max; // prevent memory leaks

   min = new Double_t[fNdim];
   max = new Double_t[fNdim];
   memcpy(min, fMin, fNdim * sizeof(Double_t));
   memcpy(max, fMax, fNdim * sizeof(Double_t));
}


//______________________________________________________________________________
Double_t TFn::GetSave(const Double_t *xx)
{
    // Get value corresponding to X in array of fSave values

   if (fNsave <= 0) return 0;
   if (fSave == 0) return 0;
   Double_t x    = Double_t(xx[0]);
   Double_t y,dx,xmin,xmax,xlow,xup,ylow,yup;
   if (fParent && fParent->InheritsFrom(TH1::Class())) {
      //if parent is a histogram the function had been savedat the center of the bins
      //we make a linear interpolation between the saved values
      xmin = fSave[fNsave-3];
      xmax = fSave[fNsave-2];
      if (fSave[fNsave-1] == xmax) {
         TH1 *h = (TH1*)fParent;
         TAxis *xaxis = h->GetXaxis();
         Int_t bin1  = xaxis->FindBin(xmin);
         Int_t binup = xaxis->FindBin(xmax);
         Int_t bin   = xaxis->FindBin(x);
         if (bin < binup) {
            xlow = xaxis->GetBinCenter(bin);
            xup  = xaxis->GetBinCenter(bin+1);
            ylow = fSave[bin-bin1];
            yup  = fSave[bin-bin1+1];
         } else {
            xlow = xaxis->GetBinCenter(bin-1);
            xup  = xaxis->GetBinCenter(bin);
            ylow = fSave[bin-bin1-1];
            yup  = fSave[bin-bin1];
         }
         dx = xup-xlow;
         y  = ((xup*ylow-xlow*yup) + x*(yup-ylow))/dx;
         return y;
      }
   }
   Int_t np = fNsave - 3;
   xmin = Double_t(fSave[np+1]);
   xmax = Double_t(fSave[np+2]);
   dx   = (xmax-xmin)/np;
   if (x < xmin || x > xmax) return 0;
   // return a Nan in case of x=nan, otherwise will crash later
   if (TMath::IsNaN(x) ) return x; 
   if (dx <= 0) return 0;

   Int_t bin     = Int_t((x-xmin)/dx);
   xlow = xmin + bin*dx;
   xup  = xlow + dx;
   ylow = fSave[bin];
   yup  = fSave[bin+1];
   y    = ((xup*ylow-xlow*yup) + x*(yup-ylow))/dx;
   return y;
}



//______________________________________________________________________________
Double_t TFn::GradientPar(Int_t ipar, const Double_t *x, Double_t eps)
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



   fParams[ipar] = par0 + h;     f1 = func->EvalPar(x,fParams);
   fParams[ipar] = par0 - h;     f2 = func->EvalPar(x,fParams);
   fParams[ipar] = par0 + h/2;   g1 = func->EvalPar(x,fParams);
   fParams[ipar] = par0 - h/2;   g2 = func->EvalPar(x,fParams);

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
   // Compute the gradient wrt parameters
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

   for (Int_t ipar=0; ipar<fNpar; ipar++){
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
   // For a TFn with dimension larger than 1 (for example a TF2 or TF3) 
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
   return 0.0;
   //return ROOT::TFnHelper::IntegralError(this,n,a,b,params,covmat,epsilon);
}

//______________________________________________________________________________
Double_t TFn::IntegralMultiple(Int_t n, const Double_t *a, const Double_t *b, Double_t eps, Double_t &relerr)
{
   //  See more general prototype below.
   //  This interface kept for back compatibility

   Int_t nfnevl,ifail;
   Int_t minpts = 2+2*n*(n+1)+1; //ie 7 for n=1
   Int_t maxpts = 1000;
   Double_t result = IntegralMultiple(n,a,b,minpts, maxpts,eps,relerr,nfnevl,ifail);
   if (ifail > 0) {
      Warning("IntegralMultiple","failed code=%d, ",ifail);
   }
   return result;
}


//______________________________________________________________________________
Double_t TFn::IntegralMultiple(Int_t n, const Double_t *a, const Double_t *b, Int_t minpts, Int_t maxpts, Double_t eps, Double_t &relerr,Int_t &nfnevl, Int_t &ifail)
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
   // Note that this function is currently called only by TF2::Integral (n=2)
   // and TF3::Integral (n=3).
   //
   // This function computes, to an attempted specified accuracy, the value of
   // the integral over an n-dimensional rectangular region.
   //
   // Input parameters:
   //
   //    n     : Number of dimensions [2,15]
   //    a,b   : One-dimensional arrays of length >= N . On entry A[i],  and  B[i],
   //            contain the lower and upper limits of integration, respectively.
   //    minpts: Minimum number of function evaluations requested. Must not exceed maxpts.
   //            if minpts < 1 minpts is set to 2^n +2*n*(n+1) +1
   //    maxpts: Maximum number of function evaluations to be allowed.
   //            maxpts >= 2^n +2*n*(n+1) +1
   //            if maxpts<minpts, maxpts is set to 10*minpts
   //    eps   : Specified relative accuracy.
   //
   // Output parameters:
   //
   //    relerr : Contains, on exit, an estimation of the relative accuracy of the result.
   //    nfnevl : number of function evaluations performed.
   //    ifail  :
   //        0 Normal exit.  . At least minpts and at most maxpts calls to the function were performed.
   //        1 maxpts is too small for the specified accuracy eps.
   //          The result and relerr contain the values obtainable for the
   //          specified value of maxpts.
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

   ROOT::Math::WrappedMultiFunction<TFn&> wf1(*this, n);

   ROOT::Math::AdaptiveIntegratorMultiDim aimd(wf1, eps, eps, maxpts);
   aimd.SetMinPts(minpts);
   double result = aimd.Integral(a,b);
   relerr = aimd.RelError();
   nfnevl = aimd.NEval();
   ifail = 0;

   return result;
}


//______________________________________________________________________________
Bool_t TFn::IsInside(const Double_t *x) const
{
   // Return kTRUE if the point is inside the function range

   for(Int_t i = 0; i < fNdim; i++)
      if (x[i] < fMin[i] || x[i] > fMax[i]) 
         return kFALSE;

   return kTRUE;
}


//______________________________________________________________________________
void TFn::Print(Option_t *option) const
{
   // Dump this function with its attributes.

//   TFormula::Print(option);
//   if (fHistogram) fHistogram->Print(option);
}


//______________________________________________________________________________
void TFn::ReleaseParameter(Int_t ipar)
{
   // Release parameter number ipar If used in a fit, the parameter
   // can vary freely. The parameter limits are reset to 0,0.

   if (ipar < 0 || ipar > fNpar-1) return;
   SetParLimits(ipar,0,0);
}


//______________________________________________________________________________
void TFn::Save(Double_t xmin, Double_t xmax, Double_t, Double_t, Double_t, Double_t)
{
   // TODO: all of this redone, maybe not needed
   // Save values of function in array fSave
/*
   if (fSave != 0) {delete [] fSave; fSave = 0;}
   if (fParent && fParent->InheritsFrom(TH1::Class())) {
      //if parent is a histogram save the function at the center of the bins
      if ((xmin >0 && xmax > 0) && TMath::Abs(TMath::Log10(xmax/xmin) > TMath::Log10(fNpx))) {
         TH1 *h = (TH1*)fParent;
         Int_t bin1 = h->GetXaxis()->FindBin(xmin);
         Int_t bin2 = h->GetXaxis()->FindBin(xmax);
         fNsave = bin2-bin1+4;
         fSave  = new Double_t[fNsave];
         Double_t xv[1];
         InitArgs(xv,fParams);
         for (Int_t i=bin1;i<=bin2;i++) {
            xv[0]    = h->GetXaxis()->GetBinCenter(i);
            fSave[i-bin1] = EvalPar(xv,fParams);
         }
         fSave[fNsave-3] = xmin;
         fSave[fNsave-2] = xmax;
         fSave[fNsave-1] = xmax;
         return;
      }
   }
   fNsave = fNpx+3;
   if (fNsave <= 3) {fNsave=0; return;}
   fSave  = new Double_t[fNsave];
   Double_t dx = (xmax-xmin)/fNpx;
   if (dx <= 0) {
      dx = (fXmax-fXmin)/fNpx;
      fNsave--;
      xmin = fXmin +0.5*dx;
      xmax = fXmax -0.5*dx;
   }
   Double_t xv[1];
   InitArgs(xv,fParams);
   for (Int_t i=0;i<=fNpx;i++) {
      xv[0]    = xmin + dx*i;
      fSave[i] = EvalPar(xv,fParams);
   }
   fSave[fNpx+1] = xmin;
   fSave[fNpx+2] = xmax;*/
}


//______________________________________________________________________________
void TFn::SavePrimitive(std::ostream &out, Option_t *option /*= ""*/)
{
   // Save primitive as a C++ statement(s) on output stream out

   // FIXME: Reconsidering this

/*
   Int_t i;
   char quote = '"';
   out<<"   "<<std::endl;
   //if (!fMethodCall) {
   if (!fType) {
      out<<"   TFn *"<<GetName()<<" = new TFn("<<quote<<GetName()<<quote<<","<<quote<<GetTitle()<<quote<<","<<fXmin<<","<<fXmax<<");"<<std::endl;
      if (fNpx != 100) {
         out<<"   "<<GetName()<<"->SetNpx("<<fNpx<<");"<<std::endl;
      }
   } else {
      out<<"   TFn *"<<GetName()<<" = new TFn("<<quote<<"*"<<GetName()<<quote<<","<<fXmin<<","<<fXmax<<","<<GetNpar()<<");"<<std::endl;
      out<<"    //The original function : "<<GetTitle()<<" had originally been created by:" <<std::endl;
      out<<"    //TFn *"<<GetName()<<" = new TFn("<<quote<<GetName()<<quote<<","<<GetTitle()<<","<<fXmin<<","<<fXmax<<","<<GetNpar()<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetRange("<<fXmin<<","<<fXmax<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetName("<<quote<<GetName()<<quote<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetTitle("<<quote<<GetTitle()<<quote<<");"<<std::endl;
      if (fNpx != 100) {
         out<<"   "<<GetName()<<"->SetNpx("<<fNpx<<");"<<std::endl;
      }
      Double_t dx = (fXmax-fXmin)/fNpx;
      Double_t xv[1];
      InitArgs(xv,fParams);
      for (i=0;i<=fNpx;i++) {
         xv[0]    = fXmin + dx*i;
         Double_t save = EvalPar(xv,fParams);
         out<<"   "<<GetName()<<"->SetSavedPoint("<<i<<","<<save<<");"<<std::endl;
      }
      out<<"   "<<GetName()<<"->SetSavedPoint("<<fNpx+1<<","<<fXmin<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetSavedPoint("<<fNpx+2<<","<<fXmax<<");"<<std::endl;
   }

   if (TestBit(kNotDraw)) {
      out<<"   "<<GetName()<<"->SetBit(TFn::kNotDraw);"<<std::endl;
   }
   if (GetFillColor() != 0) {
      if (GetFillColor() > 228) {
         TColor::SaveColor(out, GetFillColor());
         out<<"   "<<GetName()<<"->SetFillColor(ci);" << std::endl;
      } else
         out<<"   "<<GetName()<<"->SetFillColor("<<GetFillColor()<<");"<<std::endl;
   }
   if (GetFillStyle() != 1001) {
      out<<"   "<<GetName()<<"->SetFillStyle("<<GetFillStyle()<<");"<<std::endl;
   }
   if (GetMarkerColor() != 1) {
      if (GetMarkerColor() > 228) {
         TColor::SaveColor(out, GetMarkerColor());
         out<<"   "<<GetName()<<"->SetMarkerColor(ci);" << std::endl;
      } else
         out<<"   "<<GetName()<<"->SetMarkerColor("<<GetMarkerColor()<<");"<<std::endl;
   }
   if (GetMarkerStyle() != 1) {
      out<<"   "<<GetName()<<"->SetMarkerStyle("<<GetMarkerStyle()<<");"<<std::endl;
   }
   if (GetMarkerSize() != 1) {
      out<<"   "<<GetName()<<"->SetMarkerSize("<<GetMarkerSize()<<");"<<std::endl;
   }
   if (GetLineColor() != 1) {
      if (GetLineColor() > 228) {
         TColor::SaveColor(out, GetLineColor());
         out<<"   "<<GetName()<<"->SetLineColor(ci);" << std::endl;
      } else
         out<<"   "<<GetName()<<"->SetLineColor("<<GetLineColor()<<");"<<std::endl;
   }
   if (GetLineWidth() != 4) {
      out<<"   "<<GetName()<<"->SetLineWidth("<<GetLineWidth()<<");"<<std::endl;
   }
   if (GetLineStyle() != 1) {
      out<<"   "<<GetName()<<"->SetLineStyle("<<GetLineStyle()<<");"<<std::endl;
   }
   if (GetChisquare() != 0) {
      out<<"   "<<GetName()<<"->SetChisquare("<<GetChisquare()<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetNDF("<<GetNDF()<<");"<<std::endl;
   }

   //if (GetXaxis()) GetXaxis()->SaveAttributes(out,GetName(),"->GetXaxis()");
   //if (GetYaxis()) GetYaxis()->SaveAttributes(out,GetName(),"->GetYaxis()");

   Double_t parmin, parmax;
   for (i=0;i<fNpar;i++) {
      out<<"   "<<GetName()<<"->SetParameter("<<i<<","<<GetParameter(i)<<");"<<std::endl;
      out<<"   "<<GetName()<<"->SetParError("<<i<<","<<GetParError(i)<<");"<<std::endl;
      GetParLimits(i,parmin,parmax);
      out<<"   "<<GetName()<<"->SetParLimits("<<i<<","<<parmin<<","<<parmax<<");"<<std::endl;
   }
   if (!strstr(option,"nodraw")) {
      out<<"   "<<GetName()<<"->Draw("
         <<quote<<option<<quote<<");"<<std::endl;
   }
   */
}

//______________________________________________________________________________
void TFn::SetFitResult(const ROOT::Fit::FitResult & result, const Int_t* indpar )
{
   // Set the result from the fit  
   // parameter values, errors, chi2, etc...
   // Optionally a pointer to a vector (with size fNpar) of the parameter indices in the FitResult can be passed
   // This is useful in the case of a combined fit with different functions, and the FitResult contains the global result 
   // By default it is assume that indpar = {0,1,2,....,fNpar-1}. 

   if (result.IsEmpty()) { 
      Warning("SetFitResult","Empty Fit result - nathing is set in TFn");
      return;      
   }
   if (indpar == 0 && fNpar != (int) result.NPar() ) { 
      Error("SetFitResult","Invalid Fit result passed - number of parameter is %d , different than TFn::GetNpar() = %d",fNpar,result.NPar());
      return;
   }
//   if (result.Chi2() > 0) 
//      SetChisquare(result.Chi2() );
//   else 
//      SetChisquare(result.MinFcnValue() );

   SetNDF(result.Ndf() );
   SetNumberFitPoints(result.Ndf() + result.NFreeParameters() );


   for (Int_t i = 0; i < fNpar; ++i) { 
      Int_t ipar = (indpar != 0) ? indpar[i] : i;  
      if (ipar < 0) continue;
      fParams[i] = result.Parameter(ipar);
      // in case errors are not present do not set them
      if (ipar < (int) result.Errors().size() )
         fParErrors[i] = result.Error(ipar);
   }
   //invalidate cached integral since parameters have changed
   Update();   
         
}


//______________________________________________________________________________
void TFn::SetNDF(Int_t ndf)
{
   // Set the number of degrees of freedom
   // ndf should be the number of points used in a fit - the number of free parameters

   fNDF = ndf;
}


//______________________________________________________________________________
void TFn::SetNpx(Int_t npx)
{
   // Set the number of points used to draw the function
   //
   // The default number of points along x is 100 for 1-d functions and 30 for 2-d/3-d functions
   // You can increase this value to get a better resolution when drawing
   // pictures with sharp peaks or to get a better result when using TFn::GetRandom
   // the minimum number of points is 4, the maximum is 10000000 for 1-d and 10000 for 2-d/3-d functions

   const Int_t minPx = 4;
   Int_t maxPx = 10000000;
   if (fNdim > 1) maxPx = 10000;
   if (npx >= minPx && npx <= maxPx) {
      fNpx = npx;
   } 
   else { 
      if(npx < minPx) fNpx = minPx; 
      if(npx > maxPx) fNpx = maxPx; 
      Warning("SetNpx","Number of points must be >=%d && <= %d, fNpx set to %d",minPx,maxPx,fNpx);
   } 
   Update();
}


//______________________________________________________________________________
void TFn::SetParError(Int_t ipar, Double_t error)
{
   // Set error for parameter number ipar

   if (ipar < 0 || ipar > fNpar-1) return;
   fParErrors[ipar] = error;
}


//______________________________________________________________________________
void TFn::SetParErrors(const Double_t *errors)
{
   // Set errors for all active parameters
   // when calling this function, the array errors must have at least fNpar values

   if (!errors) return;
   for (Int_t i=0;i<fNpar;i++) fParErrors[i] = errors[i];
}


//______________________________________________________________________________
void TFn::SetParLimits(Int_t ipar, Double_t parmin, Double_t parmax)
{
   // Set limits for parameter ipar.
   //
   // The specified limits will be used in a fit operation
   // when the option "B" is specified (Bounds).
   // To fix a parameter, use TFn::FixParameter

   if (ipar < 0 || ipar > fNpar-1) return;
   Int_t i;
   if (!fParMin) {fParMin = new Double_t[fNpar]; for (i=0;i<fNpar;i++) fParMin[i]=0;}
   if (!fParMax) {fParMax = new Double_t[fNpar]; for (i=0;i<fNpar;i++) fParMax[i]=0;}
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
   
   Update();
}


//______________________________________________________________________________
void TFn::SetSavedPoint(Int_t point, Double_t value)
{
   // Restore value of function saved at point

   if (!fSave) {
      fNsave = fNpx+3;
      fSave  = new Double_t[fNsave];
   }
   if (point < 0 || point >= fNsave) return;
   fSave[point] = value;
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
      b >> fNpfits;
      if (v == 1) {
         b >> fHistogram;
         delete fHistogram; fHistogram = 0;
      }
      if (v > 1) {
         if (v < 4) {
            Float_t minimum,maximum;
            b >> minimum; fMinimum =minimum;
            b >> maximum; fMaximum =maximum;
         } else {
            b >> fMinimum;
            b >> fMaximum;
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


//______________________________________________________________________________
void TFn::Update()
{
   // Called by functions such as SetRange, SetNpx, SetParameters
   // to force the deletion of the associated histogram or Integral
   // TODO: reconsider

   delete fHistogram;
   fHistogram = 0;
   if (fIntegral) {
      delete [] fIntegral; fIntegral = 0;
      delete [] fAlpha;    fAlpha    = 0;
      delete [] fBeta;     fBeta     = 0;
      delete [] fGamma;    fGamma    = 0;
   }
}


//______________________________________________________________________________
void TFn::RejectPoint(Bool_t reject)
{
   // Static function to set the global flag to reject points
   // the fgRejectPoint global flag is tested by all fit functions
   // if TRUE the point is not included in the fit.
   // This flag can be set by a user in a fitting function.
   // The fgRejectPoint flag is reset by the TH1 and TGraph fitting functions.

   fgRejectPoint = reject;
}


//______________________________________________________________________________
Bool_t TFn::RejectedPoint()
{
   // See TFn::RejectPoint above

   return fgRejectPoint;
}

//______________________________________________________________________________
Double_t TFn::Moment(Double_t n, Double_t* a, Double_t* b, const Double_t *params, Double_t epsilon)
{
   // Return nth moment of function between a and b
   // See TFn::Integral() for parameter definitions

   // wrapped function in interface for integral calculation
   // using abs value of integral 

//   TFn_EvalWrapper func(this, params, kTRUE, n); 
   ROOT::Math::AdaptiveIntegratorMultiDim aimd;

   aimd.SetFunction(*this);
   aimd.SetRelTolerance(epsilon);

   Double_t norm =  aimd.Integral(a, b);
   if (norm == 0) {
      Error("Moment", "Integral zero over range");
      return 0;
   }

   // calculate now integral of x^n f(x)
   // wrapped the member function EvalNum in  interface required by integrator using the functor class 
//   ROOT::Math::Functor1D xnfunc( &func, &TFn_EvalWrapper::EvalNMom);
//   giod.SetFunction(xnfunc);

//   Double_t res = giod.Integral(a,b)/norm;
   
   //return res;
   return 0.0;
}


//______________________________________________________________________________
Double_t TFn::CentralMoment(Double_t n, Double_t* a, Double_t* b, const Double_t *params, Double_t epsilon)
{
   // Return nth central moment of function between a and b
   // (i.e the n-th moment around the mean value)   
   //
   // See TFn::Integral() for parameter definitions
   //   Author: Gene Van Buren <gene@bnl.gov>
  
   //TFn_EvalWrapper func(this, params, kTRUE, n); 

   ROOT::Math::AdaptiveIntegratorMultiDim aimd;

   aimd.SetFunction(*this);
   aimd.SetRelTolerance(epsilon);

   Double_t norm =  aimd.Integral(a, b);
   if (norm == 0) {
      Error("Moment", "Integral zero over range");
      return 0;
   }

   // calculate now integral of xf(x)
   // wrapped the member function EvalFirstMom in  interface required by integrator using the functor class 
   //ROOT::Math::Functor1D xfunc( &func, &TFn_EvalWrapper::EvalFirstMom);
   //giod.SetFunction(xfunc);

   // estimate of mean value
   //Double_t xbar = giod.Integral(a,b)/norm;

   // use different mean value in function wrapper 
   //func.fX0 = xbar; 
   //ROOT::Math::Functor1D xnfunc( &func, &TFn_EvalWrapper::EvalNMom);
   //giod.SetFunction(xnfunc);

   //Double_t res = giod.Integral(a,b)/norm;
   return 0.0;
}


