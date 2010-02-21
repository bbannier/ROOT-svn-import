#ifndef UNITTESTSUITE_H
#define UNITTESTSUITE_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include "UnitTest.h"
#include <vector>
#include <stdexcept>

namespace UnitTesting
{

   class UnitTestSuiteError : public std::logic_error
   {
   public:
      UnitTestSuiteError(const std::string& s = "") : logic_error(s) {}
   };

   class UnitTestSuite
   {
   public:
      UnitTestSuite(const std::string& title, std::ostream* sptr = &std::cout);
      std::string getName() const;
      long getNumPassed() const;
      long getNumFailed() const;
      const std::ostream* getStream() const;
      void setStream(std::ostream* osptr);
      void addTest(UnitTest* t) throw (UnitTestSuiteError);
      void addSuite(const UnitTestSuite&);
      void run();  // Calls Test::run() repeatedly
      void intro() const;
      long report() const;
      void free();  // Deletes tests
   private:
      std::string name;
      std::ostream* osptr;
      std::vector<UnitTest*> tests;
      void reset();
      // Disallowed ops:
      UnitTestSuite(const UnitTestSuite&);
      UnitTestSuite& operator=(const UnitTestSuite&);
   };

   inline UnitTestSuite::UnitTestSuite(const std::string& title, std::ostream* sptr) : name(title)
   {
      this->osptr = sptr;
   }

   inline std::string UnitTestSuite::getName() const
   {
      return name;
   }

   inline const std::ostream* UnitTestSuite::getStream() const
   {
      return osptr;
   }

   inline void UnitTestSuite::setStream(std::ostream* sptr)
   {
      this->osptr = sptr;
   }

} // namespace UnitTesting
#endif // UNITTESTSUITE_H
