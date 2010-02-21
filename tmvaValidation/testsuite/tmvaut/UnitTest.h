#ifndef UNITTEST_H
#define UNITTEST_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <string>
#include <iostream>
#include <cassert>

// The following have underscores because
// they are macros. For consistency,
// succeed_() also has an underscore.

#define test_(cond) \
  do_test(cond, #cond, __FILE__, __LINE__)
#define fail_(str) \
  do_fail(str, __FILE__, __LINE__)

namespace UnitTesting
{

   class UnitTest
   {
   public:
      UnitTest(const std::string & name="", const std::string & filename="", std::ostream* osptr = &std::cout);
      virtual ~UnitTest(){}
      virtual void run() = 0;
      long getNumPassed() const;
      long getNumFailed() const;
      const std::ostream* getStream() const;
      void setStream(std::ostream* osptr);
      void succeed_();
      long report() const;
      virtual void reset();
      void intro() const;
      const std::string & name() const;
   protected:
      void do_test(bool cond, const std::string& lbl, const char* fname, long lineno);
      void do_fail(const std::string& lbl, const char* fname, long lineno);
   private:
      std::ostream* osptr;
      long nPass;
      long nFail;
      mutable std::string fName;
      std::string fFileName;
      // Disallowed:
      UnitTest(const UnitTest&);
      UnitTest& operator=(const UnitTest&);
   };

   inline UnitTest::UnitTest(const std::string & name,
                             const std::string & filename,
                             std::ostream* sptr) :
      fName(name),
      fFileName(filename)
   {
      this->osptr = sptr;
      nPass = nFail = 0;
   }

   inline long UnitTest::getNumPassed() const
   {
      return nPass;
   }

   inline long UnitTest::getNumFailed() const
   {
      return nFail;
   }

   inline const std::ostream* UnitTest::getStream() const
   {
      return osptr;
   }

   inline void UnitTest::setStream(std::ostream* sptr)
   {
      this->osptr = sptr;
   }

   inline void UnitTest::succeed_()
   {
      ++nPass;
   }

   inline void UnitTest::reset()
   {
      nPass = nFail = 0;
   }

} // namespace UnitTesting
#endif // UNITTEST_H
