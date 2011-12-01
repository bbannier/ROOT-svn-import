#ifndef QUARTZTEST_INCLUDED
#define QUARTZTEST_INCLUDED

#include <memory>

#include "TGQuartz.h"

#include "TestPad.h"

class TH1;

class QuartzTest {
public:
   QuartzTest(unsigned w, unsigned h);
   ~QuartzTest();

   void SetContext(CGContextRef ctx);

   void SetPadSizes(unsigned w, unsigned h);
   void Draw()const;
private:
   TGQuartz fQuartzSingleton;
   
   QuartzTest(const QuartzTest &rhs) = delete;
   QuartzTest &operator = (const QuartzTest &rhs) = delete;
   
   std::auto_ptr<TH1> fHist;
   mutable TestPad fPad;
};


#endif
