#ifndef UTREADER_H
#define UTREADER_H

// Author: E. v. Toerne,  Nov 2011, implementing unit tests by C. Rosemann
// TMVA unit tests
//
// this class acts as interface to several reader applications

#include <string>
#include <iostream>
#include <cassert>
#include <vector>

#include "TTree.h"
#include "TString.h"

#include "TMVA/Reader.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

namespace UnitTesting
{
  class utReader : public UnitTest
  {
  public:
    utReader(const char* theOption="");
    virtual ~utReader();
    
    virtual void run();
    
  protected:

  private:
     // disallow copy constructor and assignment
     utReader(const utReader&);
     utReader& operator=(const utReader&);
  };
} // namespace UnitTesting
#endif // 
