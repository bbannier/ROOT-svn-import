//////////////////////////////////////////////////////////////////////////
//
// 'SPECIAL PDFS' RooFit tutorial macro #706
// 
// Histogram based p.d.f.s and functions
//
//
//
// 07/2008 - Wouter Verkerke 
// 
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooHistPdf.h"
#include "RooHistFunc.h"
#include "TCanvas.h"
#include "RooPlot.h"
using namespace RooFit ;


void testPieceWise2()
{
  // C r e a t e   p d f   f o r   s a m p l i n g 
  // ---------------------------------------------

  RooRealVar x("x","x",0,20) ;
  RooRealVar a("a","a",.01, 0,20) ;
  RooRealVar b("b","b",-.01, -20,20) ;
  RooRealVar c("c","c",.0004, 0,20) ;
  RooPolynomial p("p","p",x,RooArgList(a,b,c)) ;



  // C r e a t e   l o w   s t a t s   h i s t o g r a m
  // ---------------------------------------------------

  // Sample 500 events from p
  x.setBins(20) ;
  RooDataSet* data1 = p.generate(x,3000) ;
  RooDataSet* dataD = p.generate(x,1000) ;
  RooDataSet* dataU = p.generate(x,5000) ;
  
  data1->Print("v");

  // Create a binned dataset with 20 bins and 500 events
  RooDataHist* hist1 = data1->binnedClone() ;
  RooDataHist* histD = dataD->binnedClone() ;
  RooDataHist* histU = dataU->binnedClone() ;

  // Represent data in dh as pdf in x
  RooHistFunc histpdf1("histpdf1","histpdf1",x,*hist1,0) ;
  RooHistFunc histpdfD("histpdfD","histpdfD",x,*histD,0) ;
  RooHistFunc histpdfU("histpdfU","histpdfU",x,*histU,0) ;

  cout << "hist pdf 1 integral = " << histpdf1.createIntegral(x)->getVal() << endl;
  cout << "hist pdf D integral = " << histpdfD.createIntegral(x)->getVal() << endl;
  cout << "hist pdf U integral = " << histpdfU.createIntegral(x)->getVal() << endl;

  // coefficients for two identical histograms
  RooRealVar alpha("alpha","alpha",1,0,10);
  RooRealVar beta("beta","beta",0,0,10);
  beta.setConstant();
  // make model out of function, using norm as expected events in extended term
  // extended term should be only thing driving alpha term
  RooRealSumPdf model("model","model",RooArgSet(histpdfD,histpdfU), RooArgSet(alpha,beta),kTRUE);

  cout << "model integral = " << model.createIntegral(x)->getVal() << endl;
  model.expectedEvents(x);
  cout << "expected events = " << model.expectedEvents(x) << endl;
  model.fitTo(*hist1);
  cout << "expected events = " << model.expectedEvents(x) << endl;

  // Plot unbinned data and histogram pdf overlaid
  RooPlot* frame1 = x.frame(Title("Low statistics histogram pdf"),Bins(100)) ;
  hist1->plotOn(frame1);
  histpdf1.plotOn(frame1, LineColor(kBlack)) ;  
  histpdfD.plotOn(frame1, LineColor(kRed)) ;  
  histpdfU.plotOn(frame1, LineColor(kGreen)) ;  
  model.plotOn(frame1, LineColor(kMagenta), LineStyle(kDotted)) ;
  
  /////////////////////////////

  cout << "ok, here we go" << endl;
  TH1* hh=0;
  //  TH1* hh = model.createHistogram("hh",x,YVar(alpha,Binning(20)));
  cout << "made hist" << hh << endl;

  TCanvas* can = new TCanvas("testPieceWise","testPieceWise",800,400) ;  
  can->Divide(2) ;
  can->cd(1) ; frame1->Draw() ;
  //  can->cd(2) ; hh->Draw("surf") ;


}
