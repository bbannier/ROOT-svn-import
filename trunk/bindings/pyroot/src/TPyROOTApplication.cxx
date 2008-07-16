// Author: Wim Lavrijsen, February 2006

// Bindings
#include "PyROOT.h"
#include "TPyROOTApplication.h"
#include "Utility.h"

// ROOT
#include "TROOT.h"
#include "TInterpreter.h"
#include "TSystem.h"
#include "TBenchmark.h"
#include "TStyle.h"
#include "TError.h"
#include "Getline.h"

// CINT
#include "Api.h"


//______________________________________________________________________________
//                        Setup interactive application
//                        =============================
//
// The TPyROOTApplication sets up the nuts and bolts for interactive ROOT use
// from python, closely following TRint. Note that not everything is done here,
// some bits (such as e.g. the use of exception hook for shell escapes) are more
// easily done in python and you'll thus find them ROOT.py
//
// The intended use of this class is from python only. It is used by default in
// ROOT.py, so if you do not want to have a TApplication derived object created
// for you, you'll need to load libPyROOT.so instead.
//
// The static InitXYZ functions are used in conjunction with TPyROOTApplication
// in ROOT.py, but they can be used independently.


//- data ---------------------------------------------------------------------
ClassImp(PyROOT::TPyROOTApplication)


//- constructors/destructor --------------------------------------------------
PyROOT::TPyROOTApplication::TPyROOTApplication(
   const char* acn, int* argc, char** argv, bool bLoadLibs ) :
      TApplication( acn, argc, argv )
{
// Create a TApplication derived for use with interactive ROOT from python. A
// set of standard, often used libs is loaded if bLoadLibs is true (default).

   if ( bLoadLibs )   // note that this section could be programmed in python
   {
   // follow TRint to minimize differences with CINT
      ProcessLine( "#include <iostream>", kTRUE );
      ProcessLine( "#include <_string>",  kTRUE ); // for std::string iostream.
      ProcessLine( "#include <vector>",   kTRUE ); // needed because they're used within the
      ProcessLine( "#include <pair>",     kTRUE ); //  core ROOT dicts and CINT won't be able
                                                   //  to properly unload these files

   // allow the usage of ClassDef and ClassImp in interpreted macros
      ProcessLine( "#include <RtypesCint.h>", kTRUE );

   // disallow the interpretation of Rtypes.h, TError.h and TGenericClassInfo.h
      ProcessLine( "#define ROOT_Rtypes 0", kTRUE );
      ProcessLine( "#define ROOT_TError 0", kTRUE );
      ProcessLine( "#define ROOT_TGenericClassInfo 0", kTRUE );

   // following RINT, these are now commented out (rely on auto-loading)
   //   // the following libs are also useful to have, make sure they are loaded...
   //      gROOT->LoadClass("TMinuit",     "Minuit");
   //      gROOT->LoadClass("TPostScript", "Postscript");
   //      gROOT->LoadClass("THtml",       "Html");
   }

// save current interpreter context
   gInterpreter->SaveContext();
   gInterpreter->SaveGlobalsContext();

// prevent crashes on accessing histor
   Gl_histinit( (char*)"-" );

// prevent ROOT from exiting python
   SetReturnFromRun( kTRUE );
}


//- static public members ----------------------------------------------------
Bool_t PyROOT::TPyROOTApplication::CreatePyROOTApplication( Bool_t bLoadLibs )
{
// Create a TPyROOTApplication. Returns false if gApplication is not null.

   if ( ! gApplication ) {
   // retrieve arg list from python, translate to raw C, pass on
      PyObject* argl = PySys_GetObject( const_cast< char* >( "argv" ) );

      int argc = argl ? PyList_Size( argl ) : 1;
      char** argv = new char*[ argc ];
      for ( int i = 1; i < argc; ++i )
         argv[ i ] = PyString_AS_STRING( PyList_GET_ITEM( argl, i ) );
      argv[ 0 ] = Py_GetProgramName();

      gApplication = new TPyROOTApplication( "PyROOT", &argc, argv, bLoadLibs );
      delete[] argv;     // TApplication ctor has copied argv, so done with it

      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TPyROOTApplication::InitROOTGlobals()
{
// Setup the basic ROOT globals gBenchmark, gStyle, gProgname, if not already
// set. Always returns true.

   if ( ! gBenchmark ) gBenchmark = new TBenchmark();
   if ( ! gStyle ) gStyle = new TStyle();

   if ( ! gProgName )              // should have been set by TApplication
      gSystem->SetProgname( Py_GetProgramName() );

   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TPyROOTApplication::InitCINTMessageCallback()
{
// Install CINT message callback which will turn CINT error message into
// python exceptions. Always returns true.

   G__set_errmsgcallback( (void*)&Utility::ErrMsgCallback );
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TPyROOTApplication::InitROOTMessageCallback()
{
// Install ROOT message handler which will turn ROOT error message into
// python exceptions. Always returns true.

   SetErrorHandler( (ErrorHandlerFunc_t)&Utility::ErrMsgHandler );
   return kTRUE;
}
