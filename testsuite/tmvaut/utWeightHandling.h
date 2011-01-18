#ifndef UTWEIGHTHANDLING_H
#define UTWEIGHTHANDLING_H

// Author: E. v. Toerne,  Jan 2011
// TMVA unit tests
//
// this class tests general event weight handling in factory and in the test tree 
//

#include <string>
#include <iostream>
#include <cassert>
#include <vector>

#include "TTree.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

namespace UnitTesting
{
   class utWeightHandling : public UnitTest
   {
   public:
      utWeightHandling(const char* theOption="");
      virtual ~utWeightHandling();
      virtual void run();
      
   protected:
      virtual TTree* create_Tree(const char* opt="");
      virtual bool operateSingleFactory(const char* factoryname, const char* opt="",  float ratio0=0, float ratio1=0, float ratio2=0);

   private:
      // disallow copy constructor and assignment
      utWeightHandling(const utWeightHandling&);
      utWeightHandling& operator=(const utWeightHandling&);
   };
} // namespace UnitTesting
#endif // 
