#include "QuartzTest.h"

#include "TH1.h"

QuartzTest::QuartzTest()
               : fQuartzSingleton("TGQuartz", "quartz implementation of X11")
{
}

void do_test()
{
   TH1F * hist = new TH1F("a", "b", 100, -1., 1.);
   hist->FillRandom("gaus");
}