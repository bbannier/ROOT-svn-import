#ifndef UTFACTORY_H
#define UTFACTORY_H

// Author: E. v. Toerne,  Nov 2011, implementing unit tests by C. Rosemann
// TMVA unit tests
//
// this class acts as interface to several factory applications

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
   class utFactory : public UnitTest
   {
   public:
      utFactory(const char* theOption="");
      virtual ~utFactory();
      virtual void run();
      
   protected:
      virtual TTree* create_Tree(const char* opt="");
      virtual bool operateSingleFactory(const char* factoryname, const char* opt="");
      virtual bool addEventsToFactoryByHand(const char* factoryname, const char* opt="");

   private:
      // disallow copy constructor and assignment
      utFactory(const utFactory&);
      utFactory& operator=(const utFactory&);
   };
} // namespace UnitTesting
#endif // 
