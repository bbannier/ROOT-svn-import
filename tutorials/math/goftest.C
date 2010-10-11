// ------------------------------------------------------------------------
//
// GoFTest tutorial macro
// 
// Using Anderson-Darling and Kolmogorov-Smirnov goodness of fit tests
// 1 sample test is performed comparing data with a log-normal distribution
// and a 2 sample test is done comparing two gaussian data sets. 
//
//
// Author:   Bartolomeu Rabacal    6/2010 
// 
// ------------------------------------------------------------------------

#include <cassert>
#include "TCanvas.h"
#include "TPaveText.h"
#include "TH1.h"
#include "TF1.h"
#include "Math/GoFTest.h"
#include "Math/Functor.h"
#include "Math/WrappedTF1.h"
#include "TRandom3.h"
#include "Math/DistFunc.h"

// #define TEST_ERROR_MESSAGE

void goftest() {

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   // C a s e  1 :  T e s t  G o o d n e s s  o f  F i t  f o r  a  l o g N o r m a l  r a n d o m  s a m p l e  ( i n b u i l t  d i s t r i b u t i o n )
   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   
   UInt_t nEvents1 = 1000;

   Double_t* sample1 = new Double_t[nEvents1];
   
   TF1* lnPdf = new TF1("logNormal","ROOT::Math::lognormal_pdf(x, [0], [1])", 0, 500);
   // Set the lognormal parameters (m and s) 
   lnPdf->SetParameters(5.0,2.0);

   TH1D* h1smp = new TH1D("h1smp", "LogNormal distribution histogram", 100, 0, 500);
   h1smp->SetStats(kFALSE);
   
   for (UInt_t i = 0; i < nEvents1; ++i) { 
      Double_t data = lnPdf->GetRandom();
      sample1[i] = data;
      h1smp->Fill(data);
   }
   // Normalize correctly the histogram
   h1smp->Scale(ROOT::Math::lognormal_cdf(500., 5., 2.) / nEvents1, "width");

   TCanvas* c = new TCanvas("c","1-Sample and 2-Sample GoF Tests");
   c->Divide(1, 3);
   c->cd(1);
   h1smp->Draw();
   h1smp->SetLineColor(kBlue);
   lnPdf->SetNpx(100); // Use same points as histo for drawing
   lnPdf->SetLineColor(kRed);
   lnPdf->Draw("SAME");
      
   // -----------------------------------------
   // C r e a t e   G o F T e s t  o b j e c t 
   // -----------------------------------------
   
   ROOT::Math::GoFTest* goftest_1 = new ROOT::Math::GoFTest(nEvents1, sample1, ROOT::Math::GoFTest::kLogNormal);
      
   /* Possible calls for the 1-sample Anderson - DarlingTest test */
   /*-------------------------------------------------------------*/
   
   /* a) Returning the Anderson-Darling standardized test statistic and the p-value by homonimous method call */
   Double_t pvalueAD_1;
   Double_t A2_1;
   goftest_1->AndersonDarlingTest(pvalueAD_1, A2_1); 
   
  
   /* b) Returning the p-value for the Anderson-Darling test statistic and the p-value by functor call */
   Double_t pvalueAD_2;
   Double_t A2_2;
   (*goftest_1)(ROOT::Math::GoFTest::kAD, pvalueAD_2, A2_2);
   
   assert(A2_1 == A2_2);
   assert(pvalueAD_1 == pvalueAD_2);
   
   /* Rebuild the test using the default 1-sample constructor */
   delete goftest_1;
   goftest_1 = new ROOT::Math::GoFTest(nEvents1, sample1); // User must then input a distribution type option
   goftest_1->SetDistribution(ROOT::Math::GoFTest::kLogNormal);
   
   
   /* Possible calls for the 1-sample Kolmogorov - Smirnov test */
   /*-----------------------------------------------------------*/              
       
   /* a) Returning the Kolmogorov-Smirnov standardized test statistic and the p-value by homonimous method call */
   Double_t pvalueKS_1;
   Double_t Dn_1;
   goftest_1->KolmogorovSmirnovTest(pvalueKS_1, Dn_1);
   
   /* b) Returning the Kolmogorov-Smirnov standardized test statistic and the p-value by functor call */
   Double_t pvalueKS_2;
   Double_t Dn_2;
   (*goftest_1)(ROOT::Math::GoFTest::kKS, pvalueKS_2, Dn_2);
   
   assert(Dn_1 == Dn_2);
   assert(pvalueKS_1 == pvalueKS_2);
   
   /* Valid but incorrect calls for the 2-sample methods of the 1-samples constucted goftest_1 */
#ifdef TEST_ERROR_MESSAGE
   Double_t pvalueAD, pvalueKS; 
   Double_t A2, Dn;
   goftest_1->KolmogorovSmirnov2SamplesTest(pvalueKS, Dn); // Issues error message 
   (*goftest_1)(ROOT::Math::GoFTest::kAD2s, pvalueAD, A2); // Issues error message
   assert(pvalueAD == pvalueKS);
   assert(A2 == Dn);
#endif
  
   TPaveText* pt1 = new TPaveText(0.58, 0.6, 0.88, 0.80, "brNDC");
   Char_t str1[50];
   sprintf(str1, "p-value for A-D 1-smp test: %f", pvalueAD_1);
   pt1->AddText(str1);
   pt1->SetFillColor(18);
   pt1->SetTextFont(20);
   pt1->SetTextColor(4);
   Char_t str2[50];
   sprintf(str2, "p-value for K-S 1-smp test: %f", pvalueKS_1);
   pt1->AddText(str2);
   pt1->Draw();
   
   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   // C a s e  2 :  T e s t  G o o d n e s s  o f  F i t  f o r  2  G a u s s i a n  r a n d o m  s a m p l e s  ( i n b u i l t  d i s t r i b u t i o n )
   // ---------------------------------------------------------------------------------------------------------------------------------------------------------

   UInt_t nEvents2 = 2000;

   Double_t* sample2 = new Double_t[nEvents2];

   TH1D* h2smps_1 = new TH1D("h2smps_1", "Gaussian distribution histograms", 100, 0, 500);
   h2smps_1->SetStats(kFALSE);   
   
   TH1D* h2smps_2 = new TH1D("h2smps_2", "Gaussian distribution histograms", 100, 0, 500);
   h2smps_2->SetStats(kFALSE);
   
   TRandom3 r;
   for (UInt_t i = 0; i < nEvents1; ++i) { 
      Double_t data = r.Gaus(300, 50);
      sample1[i] = data;
      h2smps_1->Fill(data);
   }
   h2smps_1->Scale(1. / nEvents1, "width");
   c->cd(2);
   h2smps_1->Draw();
   h2smps_1->SetLineColor(kBlue);
   
   for (UInt_t i = 0; i < nEvents2; ++i) { 
      Double_t data = r.Gaus(300, 50);
      sample2[i] = data;
      h2smps_2->Fill(data);
   }
   h2smps_2->Scale(1. / nEvents2, "width");
   h2smps_2->Draw("SAME");
   h2smps_2->SetLineColor(kRed);

   // -----------------------------------------
   // C r e a t e   G o F T e s t  o b j e c t 
   // -----------------------------------------
   
   ROOT::Math::GoFTest* goftest_2 = new ROOT::Math::GoFTest(nEvents1, sample1, nEvents2, sample2);
   
   /* Possible calls for the 2-sample Anderson - DarlingTest test */
   /*-------------------------------------------------------------*/
   
   /* a) Returning the Anderson-Darling standardized test statistic and the p-value by homonimous method call */
   goftest_2->AndersonDarling2SamplesTest(pvalueAD_1, A2_1); 
  
   /* b) Returning the p-value for the Anderson-Darling test statistic and the p-value by functor call */
   (*goftest_2)(ROOT::Math::GoFTest::kAD2s, pvalueAD_2, A2_2);
   
   assert(pvalueAD_1 == pvalueAD_2);
   assert(A2_1 == A2_2);
   
   /* Possible calls for the 2-sample Kolmogorov - Smirnov test */
   /*-----------------------------------------------------------*/              
       
   /* a) Returning the Kolmogorov-Smirnov standardized test statistic and the p-value by homonimous method call */
   goftest_2-> KolmogorovSmirnov2SamplesTest(pvalueKS_1, Dn_1);
   
   /* b) Returning the Kolmogorov-Smirnov standardized test statistic and the p-value by functor call */
   (*goftest_2)(ROOT::Math::GoFTest::kKS2s, pvalueKS_2, Dn_2);
   
   assert(pvalueKS_1 == pvalueKS_2);
   assert(Dn_1 == Dn_2);

   /* Valid but incorrect calls for the 1-sample methods of the 2-samples constucted goftest_2 */
#ifdef TEST_ERROR_MESSAGE
   goftest_2->AndersonDarlingTest(pvalueAD, A2); // Issues error message
   (*goftest_2)(ROOT::Math::GoFTest::kKS, pvalueKS, Dn); // Issues error message
   assert(pvalueAD == pvalueKS);
   assert(A2 == Dn);
#endif
   
   TPaveText* pt2 = new TPaveText(0.13, 0.6, 0.43, 0.8, "brNDC");
   sprintf(str1, "p-value for A-D 2-smps test: %f", pvalueAD_1);
   pt2->AddText(str1);
   pt2->SetFillColor(18);
   pt2->SetTextFont(20);
   pt2->SetTextColor(4);
   sprintf(str2, "p-value for K-S 2-smps test: %f", pvalueKS_1);
   pt2-> AddText(str2);
   pt2-> Draw();
   
      
   // -----------------------------------------------------------------------------------------------------------------------------------------------------
   // C a s e  3 :  T e s t  G o o d n e s s  o f  F i t  f o r  L a n d a u  r a n d o m  s a m p l e  ( u s e r  i n p u t  d i s t r i b u t i o n )
   // -----------------------------------------------------------------------------------------------------------------------------------------------------
   
   UInt_t nEvents3 = 1000;
   
   Double_t* sample3 = new Double_t[nEvents3];
   
   Double_t low = -9., up = 50., mpv = 3., sigma = 1.;
   
   TF1* landauPdf = new TF1("Landau","ROOT::Math::landau_pdf(x, [0], [1])", low, up);
   landauPdf->SetParameters(mpv, sigma);
   
   TH1D* h2smp = new TH1D("h2smp", "Landau distribution histogram", 100, low, up);
   h2smp->SetStats(kFALSE);
   
   for (UInt_t i = 0; i < nEvents3; ++i) { 
      Double_t data = landauPdf->GetRandom();
      sample3[i] = data;
      h2smp->Fill(sample3[i]);
   }
   // Normalize correctly the histogram
   h2smp->Scale((ROOT::Math::landau_cdf(up, mpv, sigma) - ROOT::Math::landau_cdf(low, mpv, sigma)) / nEvents3, "width");
   c->cd(3);
   h2smp->Draw();
   h2smp->SetLineColor(kBlue);
   landauPdf->SetNpx(100); // Use same points as histogram for drawing
   landauPdf->SetLineColor(kRed);
   landauPdf->Draw("SAME");
      

   // -------------------------------------------
   // C r e a t e   G o F T e s t  o b j e c t s 
   // -------------------------------------------
   
   /* Possible constructors for the user input distribution */
   /*-------------------------------------------------------*/
   
   Double_t min = TMath::MinElement(nEvents3, sample3);
   Double_t max = TMath::MaxElement(nEvents3, sample3);
   Double_t xmin = 1.1 * min; // Distribution's lower boundary below data minimum
   Double_t xmax = 1.1 * max; // Distribution's upper boundary above data maximum
   
   /* a) User input PDF */
   ROOT::Math::WrappedTF1 pdf(*landauPdf);
   
   ROOT::Math::GoFTest* goftest_3a = new ROOT::Math::GoFTest(nEvents3, sample3, pdf, ROOT::Math::GoFTest::kPDF, xmin, xmax); // Need to specify an interval
   
   /* b) User input CDF */
   TF1* landauCdf = new TF1("LandauI","ROOT::Math::landau_cdf(x, [0], [1])", low, up);
   landauCdf->SetParameters(mpv, sigma);
   ROOT::Math::WrappedTF1 cdf(*landauCdf);
   
   ROOT::Math::GoFTest* goftest_3b = new ROOT::Math::GoFTest(nEvents3, sample3, cdf, ROOT::Math::GoFTest::kCDF, xmin, xmax);  // Need to specify an interval 

   /* Returning the Anderson-Darling standardized test statistic and the p-value */
   goftest_3a->AndersonDarlingTest(pvalueAD_1, A2_1);
   goftest_3b->AndersonDarlingTest(pvalueAD_2, A2_2);
   
   /* Checking consistency between both tests */ 
   std::cout << " \n\nTEST with LANDAU distribution:\t";
   if (TMath::Abs(pvalueAD_1 - pvalueAD_2) > 1.E-1 * pvalueAD_2) { 
      std::cout << "FAILED " << std::endl;
      Error("goftest","Error in comparing testing using Landau and Landau CDF");
      std::cerr << " pvalues are " << pvalueAD_1 << "  " << pvalueAD_2 << std::endl;
   }
   else 
      std::cout << "OK ( pvalues ~ " << TMath::FloorNint(pvalueAD_2 * 10.) / 10. << "  )" << std::endl;
   
   /* Returning the Kolmogorov-Smirnov standardized test statistic and the p-value */
   goftest_3b->KolmogorovSmirnovTest(pvalueKS_1, Dn_1);
   
   TPaveText* pt3 = new TPaveText(0.58, 0.6, 0.88, 0.80, "brNDC");
   sprintf(str1, "p-value for A-D 1-smp test: %f", pvalueAD_1);
   pt3->AddText(str1);
   pt3->SetFillColor(18);
   pt3->SetTextFont(20);
   pt3->SetTextColor(4);
   sprintf(str2, "p-value for K-S 1-smp test: %f", pvalueKS_1);
   pt3-> AddText(str2);
   pt3-> Draw();
}
