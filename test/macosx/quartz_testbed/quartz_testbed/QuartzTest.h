#ifndef QUARTZTEST_INCLUDED
#define QUARTZTEST_INCLUDED

#include "TGQuartz.h"

//#include "Test"

class QuartzTest {
public:
   QuartzTest();

   void SetPadSizes(unsigned w, unsigned h);
   void Draw()const;
private:
   TGQuartz fQuartzSingleton;
};


#endif
