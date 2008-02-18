// @(#)root/hist:$Id$
// Author: Christophe.Delaere@cern.ch   21/08/2002

///////////////////////////////////////////////////////////////////////////
//
// TConfidenceLevel
//
// Class to compute 95% CL limits
//
///////////////////////////////////////////////////////////////////////////

/*************************************************************************
 * C.Delaere                                                             *
 * adapted from the mclimit code from Tom Junk                           *
 * see http://cern.ch/thomasj/searchlimits/ecl.html                      *
 *************************************************************************/

#include "TConfidenceLevel.h"
#include "TH1F.h"
#include "TMath.h"
#include "Riostream.h"

#include <vector>

ClassImp(TConfidenceLevel)

Double_t const TConfidenceLevel::fgMCLM2S = 0.025;
Double_t const TConfidenceLevel::fgMCLM1S = 0.16;
Double_t const TConfidenceLevel::fgMCLMED = 0.5;
Double_t const TConfidenceLevel::fgMCLP1S = 0.84;
Double_t const TConfidenceLevel::fgMCLP2S = 0.975;
// LHWG "one-sided" definition
Double_t const TConfidenceLevel::fgMCL3S1S = 2.6998E-3;
Double_t const TConfidenceLevel::fgMCL5S1S = 5.7330E-7;
// the other definition (not chosen by the LHWG)
Double_t const TConfidenceLevel::fgMCL3S2S = 1.349898E-3;
Double_t const TConfidenceLevel::fgMCL5S2S = 2.866516E-7;


//______________________________________________________________________________
TConfidenceLevel::TConfidenceLevel()
{
   // Default constructor

   fStot = 0;
   fBtot = 0;
   fDtot = 0;
   fTSD  = 0;
   fTSB  = 0;
   fTSS  = 0;
   fLRS  = 0;
   fLRB  = 0;
   fNMC  = 0;
   fNNMC = 0;
   fISS  = 0;
   fISB  = 0;
   fMCL3S = fgMCL3S1S;
   fMCL5S = fgMCL5S1S;
}


//______________________________________________________________________________
TConfidenceLevel::TConfidenceLevel(Int_t mc, bool onesided)
{
   // a constructor that fix some conventions:
   // mc is the number of Monte Carlo experiments
   // while onesided specifies if the intervals are one-sided or not.

   fStot = 0;
   fBtot = 0;
   fDtot = 0;
   fTSD  = 0;
   fTSB  = 0;
   fTSS  = 0;
   fLRS  = 0;
   fLRB  = 0;
   fNMC  = mc;
   fNNMC = mc;
   fISS  = new Int_t[mc];
   fISB  = new Int_t[mc];
   fMCL3S = onesided ? fgMCL3S1S : fgMCL3S2S;
   fMCL5S = onesided ? fgMCL5S1S : fgMCL5S2S;
}


//______________________________________________________________________________
TConfidenceLevel::~TConfidenceLevel()
{
   // The destructor

   if (fISS)
      delete[]fISS;
   if (fISB)
      delete[]fISB;
   if (fTSB)
      delete[]fTSB;
   if (fTSS)
      delete[]fTSS;
   if (fLRS)
      delete[]fLRS;
   if (fLRB)
      delete[]fLRB;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetExpectedStatistic_b(Int_t sigma) const
{
   // Get the expected statistic value in the background only hypothesis

   switch (sigma) {
   case -2:
      return (-2 *((fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP2S)))]]) - fStot));
   case -1:
      return (-2 *((fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP1S)))]]) - fStot));
   case 0:
      return (-2 *((fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLMED)))]]) - fStot));
   case 1:
      return (-2 *((fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM1S)))]]) - fStot));
   case 2:
      return (-2 *((fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM2S)))]]) - fStot));
   default:
      return 0;
   }
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetExpectedStatistic_sb(Int_t sigma) const
{
   // Get the expected statistic value in the signal plus background hypothesis

   switch (sigma) {
   case -2:
      return (-2 *((fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP2S)))]]) - fStot));
   case -1:
      return (-2 *((fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP1S)))]]) - fStot));
   case 0:
      return (-2 *((fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLMED)))]]) - fStot));
   case 1:
      return (-2 *((fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM1S)))]]) - fStot));
   case 2:
      return (-2 *((fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM2S)))]]) - fStot));
   default:
      return 0;
   }
}


//______________________________________________________________________________
Double_t TConfidenceLevel::CLb(bool use_sMC) const
{
   // Get the Confidence Level for the background only

   Double_t result = 0;
   switch (use_sMC) {
   case kFALSE:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSD)
               result = (Double_t(i + 1)) / fNMC;
         return result;
      }
   case kTRUE:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSD)
               result += (1 / (fLRS[fISS[i]] * fNMC));
         return result;
      }
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::CLsb(bool use_sMC) const
{
   // Get the Confidence Level for the signal plus background hypothesis

   Double_t result = 0;
   switch (use_sMC) {
   case kFALSE:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSD)
               result += (fLRB[fISB[i]]) / fNMC;
         return result;
      }
   case kTRUE:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSD)
               result = i / fNMC;
         return result;
      }
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::CLs(bool use_sMC) const
{
   // Get the Confidence Level defined by CLs = CLsb/CLb.
   // This quantity is stable w.r.t. background fluctuations.

   Double_t clb = CLb(kFALSE);
   Double_t clsb = CLsb(use_sMC);
   if(clb==0) { cout << "Warning: clb = 0 !" << endl; return 0;}
   else return clsb/clb;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetExpectedCLsb_b(Int_t sigma) const
{
   // Get the expected Confidence Level for the signal plus background hypothesis
   // if there is only background.

   double sum = 0; 
   Double_t result = 0;
   switch (sigma) {
   case -2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP2S)))]])
               result += fLRB[fISB[i]] / fNMC;
         return result;
      }
   case -1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP1S)))]])
               result += fLRB[fISB[i]] / fNMC;
         return result;
      }
   case 0:
      {
         for (Int_t i = 0; i < fNMC; i++)
               sum += fLRB[fISB[i]] / fNMC;
         std::cout << "(sumLR = " << sum << ") ";

         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLMED)))]])
               result += fLRB[fISB[i]] / fNMC;
         return result;
      }
   case 1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM1S)))]])
               result += fLRB[fISB[i]] / fNMC;
         return result;
      }
   case 2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM2S)))]])
               result += fLRB[fISB[i]] / fNMC;
         return result;
      }
   default:
      return 0;
   }
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetExpectedCLb_sb(Int_t sigma) const
{
   // Get the expected Confidence Level for the background only
   // if there is signal and background.

   double sum = 0; 
   Double_t result = 0;
   switch (sigma) {
   case 2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP2S)))]])
               result += fLRS[fISS[i]] / fNMC;
         return result;
      }
   case 1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP1S)))]])
               result += fLRS[fISS[i]] / fNMC;
         return result;
      }
   case 0:
      {
         for (Int_t i = 0; i < fNMC; i++)
               sum += fLRS[fISS[i]] / fNMC;
         std::cout << "(sumLR = " << sum << ") ";
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLMED)))]])
               result += fLRS[fISS[i]] / fNMC;
         return result;
      }
   case -1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM1S)))]])
               result += fLRS[fISS[i]] / fNMC;
         return result;
      }
   case -2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSS[fISS[i]] <= fTSS[fISS[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM2S)))]])
               result += fLRS[fISS[i]] / fNMC;
         return result;
      }
   default:
      return 0;
   }
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetExpectedCLb_b(Int_t sigma) const
{
   // Get the expected Confidence Level for the background only
   // if there is only background.

   Double_t result = 0;
   switch (sigma) {
   case 2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM2S)))]])
               result = (i + 1) / double (fNMC);
         return result;
      }
   case 1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLM1S)))]])
               result = (i + 1) / double (fNMC);
         return result;
      }
   case 0:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLMED)))]])
               result = (i + 1) / double (fNMC);
         return result;
      }
   case -1:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP1S)))]])
               result = (i + 1) / double (fNMC);
         return result;
      }
   case -2:
      {
         for (Int_t i = 0; i < fNMC; i++)
            if (fTSB[fISB[i]] <= fTSB[fISB[TMath::Min((Int_t) fNMC,(Int_t) TMath::Max((Int_t) 1,(Int_t) (fNMC * fgMCLP2S)))]])
               result = (i + 1) / double (fNMC);
         return result;
      }
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetAverageCLsb() const
{
   // Get average CLsb.

   Double_t result = 0;
   Double_t psumsb = 0;
   for (Int_t i = 0; i < fNMC; i++) {
      psumsb += fLRB[fISB[i]] / fNMC;
      result += psumsb / fNMC;
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::GetAverageCLs() const
{
   // Get average CLs.

   Double_t result = 0;
   Double_t psumsb = 0;
   for (Int_t i = 0; i < fNMC; i++) {
      psumsb += fLRB[fISB[i]] / fNMC;
      result += ((psumsb / fNMC) / ((i + 1) / fNMC));
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::Get3sProbability() const
{
   // Get 3s probability.

   Double_t result = 0;
   Double_t psumbs = 0;
   for (Int_t i = 0; i < fNMC; i++) {
      psumbs += 1 / (Double_t) (fLRS[(fISS[(Int_t) (fNMC - i)])] * fNMC);
      if (psumbs <= fMCL3S)
         result = i / fNMC;
   }
   return result;
}


//______________________________________________________________________________
Double_t TConfidenceLevel::Get5sProbability() const
{
   // Get 5s probability.

   Double_t result = 0;
   Double_t psumbs = 0;
   for (Int_t i = 0; i < fNMC; i++) {
      psumbs += 1 / (Double_t) (fLRS[(fISS[(Int_t) (fNMC - i)])] * fNMC);
      if (psumbs <= fMCL5S)
         result = i / fNMC;
   }
   return result;
}


//______________________________________________________________________________
void  TConfidenceLevel::Draw(const Option_t* opt)
{
   // Display sort of a "canonical" -2lnQ plot.
   // This results in a plot with 2 elements:
   // - The histogram of -2lnQ for background hypothesis (full)
   // - The histogram of -2lnQ for signal and background hypothesis (dashed)
   // The 2 histograms are respectively named b_hist and sb_hist.

   TH1F h("TConfidenceLevel_Draw","",50,0,0);
   Int_t i;
   for (i=0; i<fNMC; i++) {
      h.Fill(-2*(fTSB[i]-fStot));
      h.Fill(-2*(fTSS[i]-fStot));
   }
   TH1F* b_hist  = new TH1F("b_hist", "-2lnQ",50,h.GetXaxis()->GetXmin(),h.GetXaxis()->GetXmax());
   TH1F* sb_hist = new TH1F("sb_hist","-2lnQ",50,h.GetXaxis()->GetXmin(),h.GetXaxis()->GetXmax());
   for (i=0; i<fNMC; i++) {
      b_hist->Fill(-2*(fTSB[i]-fStot));
      sb_hist->Fill(-2*(fTSS[i]-fStot));
   }
   b_hist->Draw();
   sb_hist->SetLineColor(kBlue);
   sb_hist->Draw("Same");



   if (opt != 0)  return;


   // draw the LR curves 
//#ifdef LATER
   
//#endif



   NewExpCL(true);

   // LRS and LRB are sorted now
   double xmin = std::min(log(fLRS[0] ),log(fLRB[0] ) );
   double xmax = std::max(log(fLRS[int(fNMC)-1] ),log(fLRB[int(fNMC)-1] ) );

   TH1F *h_lrs = new TH1F("LRS","LRS",50,xmin,xmax);
   TH1F *h_lrb = new TH1F("LRB","LRB",50,xmin,xmax);
   for (i=0; i<fNMC; i++) {
      h_lrs->Fill(log(fLRS[i]));
      h_lrb->Fill(log(fLRB[i]));
   }
   
//   h_lrb->Draw();
   h_lrs->SetLineColor(kBlue);
//   h_lrs->Draw("Same");
      

}

void  TConfidenceLevel::NewExpCL(bool draw) 
{

   TH1D *h_clsb_b = 0;
   TH1D *h_clb_b = 0;
   TH1D *h_clsb_sb = 0;
   TH1D *h_clb_sb = 0;

   // need to sort lrs and lrss
   double * begin = &fLRS[0]; 
   double * end = begin + int(fNMC); 
   std::sort(begin, end); 
   begin = &fLRB[0]; 
   end = begin + int(fNMC); 
   std::sort(begin, end); 

   if (draw) { 
   // draw the expected CL curves 

      h_clsb_b = new TH1D("CLSB_B","CLSB_B",100,0,1);
      h_clb_b = new TH1D("CLB_B","CLB_B",100,0,1);
      h_clsb_sb = new TH1D("CLSB_SB","CLSB_SB",100,0,1);
      h_clb_sb = new TH1D("CLB_SB","CLB_SB",100,0,1);

   }

   // new calculation of expected CL 
   std::vector<double> vCLb_sb(fNMC);
   std::vector<double> vCLb_b(fNMC);
   std::vector<double> vCLsb_sb(fNMC);
   std::vector<double> vCLsb_b(fNMC);

   for (int i=0; i<fNMC; i++) {

      double clsb_b = 0;
      double clsb_sb = 0;
      double clb_sb = 0;
      double clb_b = 0;

      
      double xb = fLRB[i]; 
      double xsb = fLRS[i];


// use stl algorithm from build in CDF optaining sorting fLRB and fLRS


      double * begin = &fLRS[0]; 
      double * end = begin + int(fNMC); 
      double * posb = std::upper_bound(begin, end, xb);
      double * possb = std::upper_bound(begin, end, xsb);
      clsb_b = std::distance(begin, posb);
      clsb_sb = std::distance(begin, possb);

      begin = &fLRB[0]; 
      end = begin + int(fNMC); 
      posb = std::upper_bound(begin, end, xb);
      possb = std::upper_bound(begin, end, xsb);
      clb_b = std::distance(begin, posb);
      clb_sb = std::distance(begin, possb);

      

#ifdef DO_LOOP

      clb_b = i;
      clsb_sb = i;

      double xsb = exp( fTSS[i]-fStot ); 
      double xb =  exp( fTSB[i]-fStot ); 

      for (int j = 0;  j < fNMC; j++ )   { 
         if (fLRS[j] > xb) {
            clsb_b = j; 
            break; 
         }
      }    
      for (int j = 0;  j < fNMC; j++ )   { 
         if (fLRB[j] > xsb) {
            clb_sb = j; 
            break; 
         }
      }  
#endif  
#ifdef NOLOP
      double xsb = fTSS[i]; 
      double xb =  fTSB[i]; 
      for (int j = 0; j < fNMC; j++) {
         if (fTSS[fISS[j]] > xb) { 
            clsb_b = j; 
            break; 
         }
      }
      for (int j = 0; j < fNMC; j++) {
         if (fTSB[fISB[j]] > xsb) { 
            clb_sb = j; 
            break; 
         }
      }
   
#endif
#ifdef DEBUG
      std::cout << "xsb , xb = " << xsb << "  " << xb << std::endl; 
      std::cout << "cl values for i " << i << "  " << clb_b/fNMC << "  " << clb_sb/fNMC << "  " << clsb_b/fNMC << "  " << clsb_sb/fNMC << std::endl;
#endif 
      if (draw) { 
         h_clsb_b->Fill(clsb_b/fNMC);
         h_clsb_sb->Fill(clsb_sb/fNMC);
         h_clb_b->Fill(clb_b/fNMC);
         h_clb_sb->Fill(clb_sb/fNMC);
      }

      vCLb_sb[i] = clb_sb/fNMC;
      vCLsb_b[i] = clsb_b/fNMC;
      vCLsb_sb[i] = clsb_sb/fNMC;
      vCLb_b[i] = clb_b/fNMC;
   }

   if (draw)  {  

      h_clsb_b->Draw(); 
      h_clb_sb->SetLineColor(kBlue);
      h_clb_sb->Draw("Same");
      h_clb_b->SetLineColor(kRed);
      h_clb_b->Draw("Same");
      h_clsb_sb->SetLineColor(kGreen);
      h_clsb_sb->Draw("Same");
   }

   // calculate average and median 
   std::cout << "New Average  CLb_sb " << TMath::Mean(fNMC, &vCLb_sb.front() ) << " Median  " <<  TMath::Median(fNMC, &vCLb_sb.front() )
             << std::endl;
   std::cout << "New Average  CLsb_b " << TMath::Mean(fNMC, &vCLsb_b.front() ) << " Median  " <<  TMath::Median(fNMC, &vCLsb_b.front() )
             << std::endl;
   std::cout << "New Average  CLb_b " << TMath::Mean(fNMC, &vCLb_b.front() ) << " Median  " <<  TMath::Median(fNMC, &vCLb_b.front() )
             << std::endl;
   std::cout << "New Average  CLsb_sb " << TMath::Mean(fNMC, &vCLsb_sb.front() ) << " Median  " <<  TMath::Median(fNMC, &vCLsb_sb.front() )
             << std::endl;

}

//______________________________________________________________________________
void  TConfidenceLevel::SetTSB(Double_t * in)
{
   // Set the TSB.
   fTSB = in; 


   TMath::Sort(fNNMC, fTSB, fISB, 0); 

#ifdef DEBUG
   std::cout << "\nTSB "  << std::endl;
   for (int i = 0; i < fNNMC; i++) 
      std::cout << fTSB[fISB[i]] << "  "; 
   std::cout << std::endl;
#endif

}


//______________________________________________________________________________
void  TConfidenceLevel::SetTSS(Double_t * in)
{
   // Set the TSS.
   fTSS = in; 


   TMath::Sort(fNNMC, fTSS, fISS, 0); 

#ifdef DEBUG
   std::cout << "\nTSS "  << std::endl;
   for (int i = 0; i < fNNMC; i++) 
      std::cout << in[fISS[i]] << "  "; 
   std::cout << std::endl;
#endif
   
}

void  TConfidenceLevel::SetLRS(Double_t * in)
{
   fLRS = in; 

#ifdef DEBUG
   std::cout << "\nLRS "  << std::endl;
   for (int i = 0; i < fNNMC; i++) 
      std::cout << in[fISS[i]] << "  "; 
   std::cout << std::endl;
#endif
}

void  TConfidenceLevel::SetLRB(Double_t * in)
{
   fLRB = in; 
#ifdef DEBUG
   std::cout << "\nLRB "  << std::endl;
   for (int i = 0; i < fNNMC; i++) 
      std::cout << in[fISB[i]] << "  "; 
   std::cout << std::endl;
#endif
}
