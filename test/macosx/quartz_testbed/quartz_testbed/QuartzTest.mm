#include <iostream>

#include "TMath.h"
#include "TH1.h"
#include "TText.h"
#include "TLine.h"


#include "QuartzTest.h"

const unsigned nBins = 100;

//______________________________________________________________________________
QuartzTest::QuartzTest(unsigned w, unsigned h)
               : fQuartzSingleton("TGQuartz", "quartz implementation of X11"),
                 fHist(new TH1F("Histogram Test", "Test Histogram", nBins, -3., 3.)),
                 fPad(w, h)
{
   gVirtualX = &fQuartzSingleton;

   for (unsigned i = 0; i < nBins; ++i)
      fHist->SetBinContent(i + 1, TMath::Sin(0.01 * i));

   fPad.cd();
   fPad.SetFillColor(kCyan);
   fPad.SetFrameFillColor(kOrange);
   fPad.SetGridx(1);
   fPad.SetGridy(1);
   
   TText *text = new TText(0.,.4,"Text Example");
   text->SetTextColor(kGreen);
   text->SetTextAngle(30.);
   
   fHist->SetAxisColor(kRed);
   fHist->Draw();
   text->Draw();
   
   TLine *l1  = new TLine(-2,.52,-1,.52); l1->SetLineStyle(1);   l1->Draw();
   TLine *l2  = new TLine(-2,.54,-1,.54); l2->SetLineStyle(2);   l2->Draw();
   TLine *l3  = new TLine(-2,.56,-1,.56); l3->SetLineStyle(3);   l3->Draw();
   TLine *l4  = new TLine(-2,.58,-1,.58); l4->SetLineStyle(4);   l4->Draw();
   TLine *l5  = new TLine(-2,.62,-1,.62); l5->SetLineStyle(5);   l5->Draw();
   TLine *l6  = new TLine(-2,.64,-1,.64); l6->SetLineStyle(6);   l6->Draw();
   TLine *l7  = new TLine(-2,.66,-1,.66); l7->SetLineStyle(7);   l7->Draw();
   TLine *l8  = new TLine(-2,.68,-1,.68); l8->SetLineStyle(8);   l8->Draw();
   TLine *l9  = new TLine(-2,.72,-1,.72); l9->SetLineStyle(9);   l9->Draw();
   TLine *l10 = new TLine(-2,.74,-1,.74); l10->SetLineStyle(10); l10->Draw();
   
   TLine *lw1  = new TLine(-2,.22,-1,.22); lw1->SetLineWidth(2);   lw1->Draw();
   TLine *lw2  = new TLine(-2,.24,-1,.24); lw2->SetLineWidth(4);   lw2->Draw();
   TLine *lw3  = new TLine(-2,.26,-1,.26); lw3->SetLineWidth(6);   lw3->Draw();
   TLine *lw4  = new TLine(-2,.28,-1,.28); lw4->SetLineWidth(8);   lw4->Draw();
}

//______________________________________________________________________________
QuartzTest::~QuartzTest()
{
   //For auto_ptr's dtor.
}

//______________________________________________________________________________
void QuartzTest::SetPadSizes(unsigned w, unsigned h)
{
   fPad.cd();
   fPad.SetViewWH(w, h);
}

//______________________________________________________________________________
void QuartzTest::SetContext(CGContextRef ctx)
{
   fQuartzSingleton.SetContext(ctx);
}

//______________________________________________________________________________
void QuartzTest::Draw()const
{
   fPad.cd();
   fPad.Paint();
}

