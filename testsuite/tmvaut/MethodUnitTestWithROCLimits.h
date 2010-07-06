#ifndef METHODUNITTESTWITHROCLIMITS_H
#define METHODUNITTESTWITHROCLIMITS_H

// Author: Christoph Rosemann  Jul 2010
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

#include "TMVA/Factory.h"
#include "TMVA/MethodBase.h"
#include "UnitTest.h"

namespace UnitTesting
{
  class MethodUnitTestWithROCLimits : public UnitTest
  {
  public:
    MethodUnitTestWithROCLimits(double upLimit = 0., double lowLimit = 1.,
		   const std::string & name="", const std::string & filename="", std::ostream* osptr = &std::cout);
    virtual ~MethodUnitTestWithROCLimits();
    
    virtual void run();
    
    void init();
    
  protected:
    TTree*          theTree;

  private:
    TMVA::Factory* _factory; 
    TMVA::MethodBase* _theMethod;	
    double _upROCLimit;
    double _lowROCLimit;
    double _ROCValue;

    bool ROCIntegralWithinInterval();

    // disallow copy constructor and assignment
    MethodUnitTestWithROCLimits(const MethodUnitTestWithROCLimits&);
    MethodUnitTestWithROCLimits& operator=(const MethodUnitTestWithROCLimits&);
  };
} // namespace UnitTesting
#endif // METHODUNITTESTWITHROCLIMITS_H
