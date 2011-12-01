#include <iostream>

#include "TMath.h"
#include "TH1.h"

#include "QuartzTest.h"

const unsigned nBins = 100;

//______________________________________________________________________________
QuartzTest::QuartzTest(unsigned w, unsigned h)
               : fQuartzSingleton("TGQuartz", "quartz implementation of X11"),
                 fHist(new TH1F("test_hist", "test_hist", nBins, -3., 3.)),
                 fPad(w, h)
{
   gVirtualX = &fQuartzSingleton;

   for (unsigned i = 0; i < nBins; ++i)
      fHist->SetBinContent(i + 1, TMath::Sin(0.01 * i));

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
