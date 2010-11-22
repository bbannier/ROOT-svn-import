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


void testHistPdfGenerate()
{
  // C r e a t e   p d f   f o r   s a m p l i n g 
  // ---------------------------------------------

  RooRealVar x("x","x",0,100) ;
  RooRealVar a("a","a",.01, 0,20) ;
  RooRealVar b("b","b",-.01, -20,20) ;
  RooRealVar c("c","c",.0004, 0,20) ;
  RooPolynomial p("p","p",x,RooArgList(a,b,c)) ;

  // C r e a t e   l o w   s t a t s   h i s t o g r a m
  // ---------------------------------------------------

  // Sample 500 events from p
  x.setBins(100) ;
  RooDataSet* data1 = p.generate(x,5000) ;
  
  // Create a binned dataset with 20 bins and 500 events
  RooDataHist* hist1 = data1->binnedClone() ;

  // Represent data in dh as pdf in x
  RooHistPdf histpdf1("histpdf1","histpdf1",x,*hist1,1) ;
  RooHistPdf histpdf2("histpdf2","histpdf2",x,*hist1,0) ;

  TStopwatch w;
  TStopwatch y;
  w.Start();
  RooDataSet* newData1 = histpdf1.generate(x,50000) ;
  w.Stop();
  y.Start();
  RooDataSet* newData2 = histpdf2.generate(x,50000) ;
  y.Stop();
  w.Print();
  y.Print();
 
  // Plot unbinned data and histogram pdf overlaid
  RooPlot* frame1 = x.frame(Title("Low statistics histogram pdf"),Bins(100)) ;
  data1->plotOn(frame1);
  newData1->plotOn(frame1,MarkerColor(kRed));
  newData2->plotOn(frame1,MarkerColor(kBlue));
  histpdf1.plotOn(frame1, LineColor(kBlack), LineStyle(kDotted)) ;  
  histpdf2.plotOn(frame1, LineColor(kRed), LineStyle(kDashed)) ;  
  frame1->Draw();


}
