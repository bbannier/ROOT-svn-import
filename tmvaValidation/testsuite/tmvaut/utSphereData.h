#ifndef UTSPHEREDATA_H
#define UTSPHEREDATA_H

// Author: Eckhard von Toerne, 
// uses Christoph Rosemann's MethodUnitTest as example  Nov 2010
// TMVA unit tests
//
// this class acts as interface to create, train and evaluate the method 
// specified in the constructor
// as additional argument the limits of the ROC integral can be given 
// to determine the performance

#include <string>
#include <iostream>
#include <cassert>

#include "TTree.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

namespace UnitTesting
{
   class utSphereData : public UnitTest
   {
   public:
      utSphereData(const TString& treestring, const TString& preparestring, 
                                    const TMVA::Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
                                    double lowLimit = 0., double upLimit = 1.,
                                    std::ostream* osptr = &std::cout);
      virtual ~utSphereData();
      
      virtual void run();
      
   protected:
      
   private:
      TMVA::Factory* _factory; 
      TMVA::MethodBase* _theMethod;
      TMVA::Types::EMVA _methodType;
      TString _treeString;
      TString _prepareString;
      TString _methodTitle;
      TString _methodOption;
      
      double _upROCLimit;
      double _lowROCLimit;
      double _ROCValue;
      
      bool ROCIntegralWithinInterval();

      // disallow copy constructor and assignment
      utSphereData(const utSphereData&);
      utSphereData& operator=(const utSphereData&);
   };
} // namespace UnitTesting
#endif // METHODUNITTESTCOMPLEXDATA_H
