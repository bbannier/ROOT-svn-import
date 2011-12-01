#include "QuartzTest.h"

#include "TH1.h"

//______________________________________________________________________________
QuartzTest::QuartzTest(unsigned w, unsigned h)
               : fQuartzSingleton("TGQuartz", "quartz implementation of X11"),
                 fHist(new TH1F("test_hist", "test_hist", 100, -3., 3.)),
                 fPad(w, h)
{
   gVirtualX = &fQuartzSingleton;
   fHist->FillRandom("gaus", 1000000);
   fPad.cd();
   fHist->Draw();
}

//______________________________________________________________________________
QuartzTest::~QuartzTest()
{
   //For auto_ptr's dtor.
}

//______________________________________________________________________________
void QuartzTest::SetPadSizes(unsigned w, unsigned h)
{
   fPad.SetViewWH(w, h);
}

//______________________________________________________________________________
void QuartzTest::Draw()const
{
   fPad.Paint();
}
