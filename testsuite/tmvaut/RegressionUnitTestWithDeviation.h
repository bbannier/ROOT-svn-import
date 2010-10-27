#ifndef REGRESSIONUNITTESTWITHDEVIATION_H
#define REGRESSIONUNITTESTWITHDEVIATION_H

// Author: Christoph Rosemann  Oct 2010
// TMVA unit tests
//
// this class acts as interface to create, train and evaluate the method 
// specified in the constructor
// additionally the regression performance is evaluated

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
  class RegressionUnitTestWithDeviation : public UnitTest
  {
  public:
    RegressionUnitTestWithDeviation(const TMVA::Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
                                    double lowFullLimit = 0., double upFullLimit = 10., double low90PercentLimit = 0., double up90PercentLimit = 0.,
                                    const std::string & name="", const std::string & filename="", std::ostream* osptr = &std::cout);
    virtual ~RegressionUnitTestWithDeviation();
    
    virtual void run();
    
  protected:
    TTree*          theTree;

  private:
    TMVA::Factory* _factory; 
    TMVA::MethodBase* _theMethod;
    TMVA::Types::EMVA _methodType;
    TString _methodTitle;
    TString _methodOption;

    double _lowerFullDeviationLimit;
    double _upperFullDeviationLimit;
    double _lower90PercentDeviationLimit;
    double _upper90PercentDeviationLimit;
    
    double _theFullDeviation;
    double _the90PercentDeviation;
    
    bool DeviationWithinLimits();

    // disallow copy constructor and assignment
    RegressionUnitTestWithDeviation(const RegressionUnitTestWithDeviation&);
    RegressionUnitTestWithDeviation& operator=(const RegressionUnitTestWithDeviation&);
  };
} // namespace UnitTesting
#endif // REGRESSIONUNITTESTWITHDEVIATION_H
