// @(#)root/fit:$Id: src/MinimizerFactory.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Fri Dec 22 14:43:33 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class MinimizerFactory

#include "Fit/MinimizerFactory.h"

#define USE_PLUGIN_MANAGER

#ifdef USE_PLUGIN_MANAGER
// use PM 
#include "Math/Minimizer.h"
#include "TPluginManager.h"
#include "TRoot.h"

#else 
// all the minimizer implementation classes 
#include "Fit/Minuit2Minimizer.h"
#include "Fit/TMinuitMinimizer.h"
#include "Fit/DummyMinimizer.h"
#include "Math/GSLMinimizer.h"
#include "Math/GSLNLSMinimizer.h"
#include "Math/GSLSimAnMinimizer.h"

#endif

#include <algorithm>

//#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif

namespace ROOT { 

   namespace Fit { 

#ifndef USE_PLUGIN_MANAGER 
int GetAlgorithmIndex(const std::string & minimType, const std::string & algoType) { 
   // get index of specific algorithm for a given minimizer 

   std::string algoname = algoType; 
   std::transform(algoType.begin(), algoType.end(), algoname.begin(), (int(*)(int)) std::tolower ); 

#ifdef DEBUG
   std::cout << "get index for algorithm " << algoname << std::endl;
#endif

   if (minimType == "Minuit2" || minimType == "Minuit" ) { 
      if (algoname == "migrad") return 0; 
      if (algoname == "simplex") return 1; 
      if (algoname == "minimize" ) return 2; 
      if (algoname == "scan" ) return 3; 
      if (algoname == "fumili" ) return 4;
      return 0; 
   }
   if (minimType == "GSLMultiMin") { 
      if (algoname == "conjugatefr") return 0; 
      if (algoname == "conjugatepr") return 1; 
      if (algoname == "bfgs") return 2;
   }
   // for all other case use default value
   return 0; 
      
}
   
#endif

#ifdef USE_PLUGIN_MANAGER
ROOT::Math::Minimizer * MinimizerFactory::CreateMinimizer(const std::string & minimizerType, const std::string & algoType)  
{
   

   // create Minimizer using the PM
   TPluginHandler *h; 
   //gDebug = 3; 
   if ((h = gROOT->GetPluginManager()->FindHandler("ROOT::Math::Minimizer",minimizerType.c_str() ))) {
      if (h->LoadPlugin() == -1)
         return 0;

      // use algorithm
      const char * algo = algoType.c_str();  
      
      ROOT::Math::Minimizer * min = reinterpret_cast<ROOT::Math::Minimizer *>( h->ExecPlugin(1,algo ) ); 
#ifdef DEBUG
      std::cout << "Loaded Minimizer " << minimizerType << "  " << algoType << "  " << algo << std::endl;
#endif

      return min; 
   }
   return 0;
                                                                                          
}
#else 
Minimizer * MinimizerFactory::CreateMinimizer(const std::string & minimizerType, const std::string & algoType)  
{
   // static method to create a minimizer . 
   // not using PM so direct dependency on all libraries (Minuit, Minuit2, MathMore, etc...)
   // The default is the Minuit2 minimizer

   // should use enumerations instead of string ?  
   
   Minimizer * min = 0; 
   int algo = GetAlgorithmIndex(minimizerType,algoType);

   if (minimizerType ==  "Minuit2" )        
       min = new ROOT::Fit::Minuit2Minimizer(algo); 

   // use TMinuit
   else if (minimizerType ==  "Minuit" || minimizerType ==  "TMinuit")        
       min = new ROOT::Fit::TMinuitMinimizer(algo);        

   // use GSL minimizer 
   else if (minimizerType ==  "GSL")        
       min = new ROOT::Math::GSLMinimizer(algo);        

   else if (minimizerType ==  "GSL_NLS")        
       min = new ROOT::Math::GSLNLSMinimizer();        

   else if (minimizerType ==  "GSL_SIMAN")        
       min = new ROOT::Math::GSLSimAnMinimizer();        

   // use dummy for testing
   else if (minimizerType ==  "Dummy")        
       min = new ROOT::Fit::DummyMinimizer();        

   // DEFAULT IS MINUIT2 based on MIGRAD
   else
       min = new ROOT::Fit::Minuit2Minimizer(); 

   return min; 
}
#endif

   } // end namespace Fit

} // end namespace ROOT

