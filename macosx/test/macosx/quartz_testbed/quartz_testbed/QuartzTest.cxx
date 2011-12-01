#include "QuartzTest.h"

#include "TH1.h"

//______________________________________________________________________________
QuartzTest::QuartzTest()
               : fQuartzSingleton("TGQuartz", "quartz implementation of X11")
{
}

//______________________________________________________________________________
void QuartzTest::SetPadSizes(unsigned w, unsigned h)
{
}

//______________________________________________________________________________
void QuartzTest::Draw()const
{
}

void do_test()
{
   TH1F * hist = new TH1F("a", "b", 100, -1., 1.);
   hist->FillRandom("gaus");
}