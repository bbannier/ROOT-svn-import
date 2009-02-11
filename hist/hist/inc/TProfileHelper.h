// @(#)root/hist:$Id: TProfileHelper.h 22724 2008-03-19 09:24:06Z moneta $
// Author: David Gonzalez Maline   18/01/2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProfileHelper
#define ROOT_TProfileHelper


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProfileHelper                                                       //
//                                                                      //
// Profile helper class.                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TH1.h"
#include "TError.h"
#include "TCollection.h"
#include "THashList.h"
#include "TMath.h"

class TProfileHelper {

public:
   template <typename T>
   static void Add(T* p, const TH1 *h1,  const TH1 *h2, Double_t c1, Double_t c2=1);

   template <typename T>
   static Double_t GetBinEffectiveEntries(T* p, Int_t bin);

   template <typename T>
   static Long64_t Merge(T* p, TCollection *list);

   template <typename T>
   static T* RebinAxis(T* p, Double_t x, TAxis *axis);

   template <typename T>
   static void Scale(T* p, Double_t c1, Option_t * option);

   template <typename T> 
   static void Sumw2(T* p);

   template <typename T>
   static void LabelsDeflate(T* p, Option_t *);

   template <typename T>
   static void LabelsInflate(T* p, Option_t *);

   template <typename T>
   static void LabelsOption(T* p, Option_t *option, Option_t *ax);
};

template <typename T>
void TProfileHelper::Add(T* p, const TH1 *h1,  const TH1 *h2, Double_t c1, Double_t c2)
{
   // Performs the operation: this = c1*h1 + c2*h2

   T *p1 = (T*)h1;
   T *p2 = (T*)h2;

// Check profile compatibility
   Int_t nx = p->GetNbinsX();
   Int_t ny = p->GetNbinsY();
   Int_t nz = p->GetNbinsZ();

   if ( nx != p1->GetNbinsX() ||  nx != p2->GetNbinsX() ||
        ny != p1->GetNbinsY() ||  ny != p2->GetNbinsY() ||
        nz != p1->GetNbinsZ() ||  nz != p2->GetNbinsZ() ) {
      Error("Add","Attempt to add profiles with different number of bins");
      return;
   }

// Add statistics
   Double_t ac1 = TMath::Abs(c1);
   Double_t ac2 = TMath::Abs(c2);
   p->fEntries = ac1*p1->GetEntries() + ac2*p2->GetEntries();
   Double_t s0[TH1::kNstat], s1[TH1::kNstat], s2[TH1::kNstat];
   Int_t i;
   for (i=0;i<TH1::kNstat;i++) {s0[i] = s1[i] = s2[i] = 0;}
   p->GetStats(s0);
   p1->GetStats(s1);
   p2->GetStats(s2);
   for (i=0;i<TH1::kNstat;i++) {
      if (i == 1) s0[i] = c1*c1*s1[i] + c2*c2*s2[i]; 
      else        s0[i] = ac1*s1[i] + ac2*s2[i];
   }
   p->PutStats(s0);

// Make the loop over the bins to calculate the Addition
   Int_t bin;
   Double_t *cu1 = p1->GetW();    Double_t *cu2 = p2->GetW();
   Double_t *er1 = p1->GetW2();   Double_t *er2 = p2->GetW2();
   Double_t *en1 = p1->GetB();    Double_t *en2 = p2->GetB();
   Double_t *ew1 = p1->GetB2();   Double_t *ew2 = p2->GetB2();
   // create sumw2 per bin if not set 
   if (p->fBinSumw2.fN == 0 && (p1->fBinSumw2.fN != 0 || p2->fBinSumw2.fN != 0) ) p->Sumw2();
   // if p1 has not the sum of weight squared/bin stored use just the sum of weights  
   if (ew1 == 0) ew1 = en1;
   if (ew2 == 0) ew2 = en2; 
   for (bin =0;bin< p->fN;bin++) {
      p->fArray[bin]             = c1*cu1[bin] + c2*cu2[bin];
      p->fSumw2.fArray[bin]      = ac1*er1[bin] + ac2*er2[bin];
      p->fBinEntries.fArray[bin] = ac1*en1[bin] + ac2*en2[bin];
      if (p->fBinSumw2.fN ) p->fBinSumw2.fArray[bin]  = ac1*ac1*ew1[bin] + ac1*ac1*ew1[bin];
   }
}

template <typename T>
Double_t TProfileHelper::GetBinEffectiveEntries(T* p, Int_t bin) 
{
//            Return bin effective entries for a weighted filled Profile histogram. 
//            In case of an unweighted profile, it is equivalent to the number of entries per bin   
//            The effective entries is defined as the square of the sum of the weights divided by the 
//            sum of the weights square. 
//            TProfile::Sumw2() must be called before filling the profile with weights. 
//            Only by calling this method the  sum of the square of the weights per bin is stored. 
//  

   if (p->fBuffer) p->BufferEmpty();

   if (bin < 0 || bin >= p->fNcells) return 0;
   double sumOfWeights = p->fBinEntries.fArray[bin];
   if ( p->fBinSumw2.fN == 0) return sumOfWeights;  
   double sumOfWeightsSquare = p->fBinSumw2.fArray[bin]; 
   return ( sumOfWeightsSquare > 0 ?  sumOfWeights * sumOfWeights /   sumOfWeightsSquare : 0 ); 
}

template <typename T>
Long64_t TProfileHelper::Merge(T* p, TCollection *li) {
   //Merge all histograms in the collection in this histogram.
   //This function computes the min/max for the axes,
   //compute a new number of bins, if necessary,
   //add bin contents, errors and statistics.
   //If overflows are present and limits are different the function will fail.
   //The function returns the total number of entries in the result histogram
   //if the merge is successfull, -1 otherwise.
   //
   //IMPORTANT remark. The 2 axis x and y may have different number
   //of bins and different limits, BUT the largest bin width must be
   //a multiple of the smallest bin width and the upper limit must also
   //be a multiple of the bin width.

   if (!li) return 0;
   if (li->IsEmpty()) return (Int_t) p->GetEntries();

   TList inlist;
   TH1* hclone = (TH1*)p->Clone("FirstClone");
   R__ASSERT(hclone);
   p->BufferEmpty(1);         // To remove buffer.
   p->Reset();                // BufferEmpty sets limits so we can't use it later.
   p->SetEntries(0);
   inlist.Add(hclone);
   inlist.AddAll(li);

   TAxis newXAxis;
   TAxis newYAxis;
   TAxis newZAxis;
   Bool_t initialLimitsFound = kFALSE;
   Bool_t same = kTRUE;
   Bool_t allHaveLimits = kTRUE;

   TIter next(&inlist);
   while (TObject *o = next()) {
      T* h = dynamic_cast<T*> (o);
      if (!h) {
         Error("Add","Attempt to add object of class: %s to a %s",
               o->ClassName(),p->ClassName());
         return -1;
      }
      Bool_t hasLimits = h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax();
      allHaveLimits = allHaveLimits && hasLimits;

      if (hasLimits) {
         h->BufferEmpty();
         if (!initialLimitsFound) {
            initialLimitsFound = kTRUE;
            newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(),
                     h->GetXaxis()->GetXmax());
            if ( p->GetDimension() >= 2 )
            newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(),
                     h->GetYaxis()->GetXmax());
            if ( p->GetDimension() >= 3 )
            newZAxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(),
                     h->GetZaxis()->GetXmax());
         }
         else {
            if (!p->RecomputeAxisLimits(newXAxis, *(h->GetXaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax(),
                     h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(),
                     h->GetXaxis()->GetXmax());
            }
            if (p->GetDimension() >= 2 && !p->RecomputeAxisLimits(newYAxis, *(h->GetYaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax(),
                     h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(),
                     h->GetYaxis()->GetXmax());
            }
            if (p->GetDimension() >= 3 && !p->RecomputeAxisLimits(newZAxis, *(h->GetZaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newZAxis.GetNbins(), newZAxis.GetXmin(), newZAxis.GetXmax(),
                     h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(),
                     h->GetZaxis()->GetXmax());
            }
         }
      }
   }
   next.Reset();

   same = same && p->SameLimitsAndNBins(newXAxis, *(p->GetXaxis()));
   if ( p->GetDimension() >= 3 )
      same = same && p->SameLimitsAndNBins(newYAxis, *(p->GetYaxis()));
   if ( p->GetDimension() >= 3 )
      same = same && p->SameLimitsAndNBins(newZAxis, *(p->GetZaxis()));
   if (!same && initialLimitsFound) {
      Double_t v[] = { newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax(),
                       newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax(),
                       newZAxis.GetNbins(), newZAxis.GetXmin(), newZAxis.GetXmax() };
      p->SetBins(v);
   }

   if (!allHaveLimits) {
      // fill this histogram with all the data from buffers of histograms without limits
      while (T* h = dynamic_cast<T*> (next())) {
         if (h->GetXaxis()->GetXmin() >= h->GetXaxis()->GetXmax() && h->fBuffer) {
             // no limits
            Int_t nbentries = (Int_t)h->fBuffer[0];
            for (Int_t i = 0; i < nbentries; i++)
               if ( p->GetDimension() == 3 ) {
                  Double_t v[] = { h->fBuffer[5*i + 2], h->fBuffer[5*i + 3],
                                   h->fBuffer[5*i + 4], h->fBuffer[5*i + 5], h->fBuffer[5*i + 1] };
                  p->Fill(v);
               } else if ( p->GetDimension() == 2 ) {
                  Double_t v[] = { h->fBuffer[4*i + 2], h->fBuffer[4*i + 3],
                                   h->fBuffer[4*i + 4], h->fBuffer[4*i + 1] };
                  p->Fill(v);
               }
         }
      }
      if (!initialLimitsFound)
         return (Int_t) p->GetEntries();  // all histograms have been processed
      next.Reset();
   }

   //merge bin contents and errors
   Double_t stats[TH1::kNstat], totstats[TH1::kNstat];
   for (Int_t i=0;i<TH1::kNstat;i++) {totstats[i] = stats[i] = 0;}
   p->GetStats(totstats);
   Double_t nentries = p->GetEntries();
   Bool_t canRebin=p->TestBit(TH1::kCanRebin);
   p->ResetBit(TH1::kCanRebin); // reset, otherwise setting the under/overflow will rebin

   while ( T* h=static_cast<T*>(next()) ) {
      // process only if the histogram has limits; otherwise it was processed before
      if (h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax()) {
         // import statistics
         h->GetStats(stats);
         for (Int_t i = 0; i < TH1::kNstat; i++)
            totstats[i] += stats[i];
         nentries += h->GetEntries();

         for ( Int_t hbin = 0; hbin < h->fN; ++hbin ) {
            Int_t hbinx, hbiny, hbinz;
            h->GetBinXYZ(hbin, hbinx, hbiny, hbinz);
            Int_t pbin = p->GetBin( h->fXaxis.FindBin( h->GetXaxis()->GetBinCenter(hbinx) ),
                                    h->fYaxis.FindBin( h->GetYaxis()->GetBinCenter(hbiny) ),
                                    h->fZaxis.FindBin( h->GetZaxis()->GetBinCenter(hbinz) ) );
            if ((!same) && (h->IsBinUnderflow(hbin) || h->IsBinOverflow(hbin)) ) {
               if (h->GetW()[hbin] != 0) {
                  Error("Merge", "Cannot merge histograms - the histograms have"
                        " different limits and undeflows/overflows are present."
                        " The initial histogram is now broken!");
                  return -1;
               }
            }
            p->fArray[pbin]             += h->GetW()[hbin];
            p->fSumw2.fArray[pbin]      += h->GetW2()[hbin];
            p->fBinEntries.fArray[pbin] += h->GetB()[hbin];
            if (p->fBinSumw2.fN) { 
               if ( h->GetB2() ) p->fBinSumw2.fArray[pbin] += h->GetB2()[hbin];
               else p->fBinSumw2.fArray[pbin] += h->GetB()[hbin];
            }
         }
      }
   }
   if (canRebin) p->SetBit(TH1::kCanRebin);

   //copy merged stats
   p->PutStats(totstats);
   p->SetEntries(nentries);
   inlist.Remove(hclone);
   delete hclone;
   return (Long64_t)nentries;
}

template <typename T>
T* TProfileHelper::RebinAxis(T* p, Double_t x, TAxis *axis)
{
// Profile histogram is resized along axis such that x is in the axis range.
// The new axis limits are recomputed by doubling iteratively
// the current axis range until the specified value x is within the limits.
// The algorithm makes a copy of the histogram, then loops on all bins
// of the old histogram to fill the rebinned histogram.
// Takes into account errors (Sumw2) if any.
// The bit kCanRebin must be set before invoking this function.
//  Ex:  h->SetBit(TH1::kCanRebin);

   if (!p->TestBit(TH1::kCanRebin)) return 0;
   if (axis->GetXmin() >= axis->GetXmax()) return 0;
   if (axis->GetNbins() <= 0) return 0;

   Double_t xmin, xmax;
   if (!p->FindNewAxisLimits(axis, x, xmin, xmax))
      return 0;

   //save a copy of this histogram
   T *hold = (T*)p->Clone();
   hold->SetDirectory(0);
   //set new axis limits
   axis->SetLimits(xmin,xmax);
   if (p->fBinSumw2.fN) hold->Sumw2();

   Int_t  nbinsx = p->fXaxis.GetNbins();
   Int_t  nbinsy = p->fYaxis.GetNbins();
   Int_t  nbinsz = p->fZaxis.GetNbins();

   //now loop on all bins and refill
   p->Reset("ICE"); //reset only Integral, contents and Errors

   Double_t bx,by,bz;
   Int_t ix, iy, iz, binx, biny, binz;
   for (binz=1;binz<=nbinsz;binz++) {
      bz  = hold->GetZaxis()->GetBinCenter(binz);
      iz  = p->fZaxis.FindFixBin(bz);
      for (biny=1;biny<=nbinsy;biny++) {
         by  = hold->GetYaxis()->GetBinCenter(biny);
         iy  = p->fYaxis.FindFixBin(by);
         for (binx=1;binx<=nbinsx;binx++) {
            bx = hold->GetXaxis()->GetBinCenter(binx);
            ix  = p->fXaxis.FindFixBin(bx);

            Int_t sourceBin = hold->GetBin(binx,biny,binz);
            Int_t destinationBin = p->GetBin(ix,iy,iz);
            p->AddBinContent(destinationBin, hold->fArray[sourceBin]);
            p->fBinEntries.fArray[destinationBin] += hold->fBinEntries.fArray[sourceBin];
            p->fSumw2.fArray[destinationBin] += hold->fSumw2.fArray[sourceBin];
            if (p->fBinSumw2.fN) p->fBinSumw2.fArray[destinationBin] += hold->fBinSumw2.fArray[sourceBin];
         }
      }
   }
   return hold;
}

template <typename T>
void TProfileHelper::Scale(T* p, Double_t c1, Option_t *)
{
   Double_t ent = p->fEntries;
   p->fScaling = kTRUE;
   if (p->fBinSumw2.fN == 0) p->Sumw2();
   p->Add(p,p,c1,0);
   p->fScaling = kFALSE;
   p->fEntries = ent;
}

template <typename T>
void TProfileHelper::Sumw2(T* p)
{
   // Create structure to store sum of squares of weights per bin  *-*-*-*-*-*-*-*
   //   This is needed to compute  the correct statistical quantities  
   //    of a profile filled with weights 
   //  
   //
   //  This function is automatically called when the histogram is created
   //  if the static function TH1::SetDefaultSumw2 has been called before.

   if (!p->fgDefaultSumw2 && p->fBinSumw2.fN) {
      Warning("Sumw2","Sum of squares of weights structure already created");
      return;
   }

   p->fBinSumw2.Set(p->fNcells);

   // by default fill with the sum of weights wich are stored in fBinEntries
   for (Int_t bin=0; bin<p->fNcells; bin++) {
      p->fBinSumw2.fArray[bin] = p->fBinEntries.fArray[bin]; 
   }
}

template <typename T>
void TProfileHelper::LabelsDeflate(T* p, Option_t *ax)
{
// Reduce the number of bins for this axis to the number of bins having a label.

   TAxis *axis = p->GetXaxis();
   if (ax[0] == 'y' || ax[0] == 'Y') axis = p->GetYaxis();
   if (!axis->GetLabels()) return;
   TIter next(axis->GetLabels());
   TObject *obj;
   Int_t nbins = 0;
   while ((obj = next())) {
      if (obj->GetUniqueID()) nbins++;
   }
   if (nbins < 2) nbins = 2;
   T *hold = (T*)p->Clone();
   hold->SetDirectory(0);

   Int_t  nbxold = p->fXaxis.GetNbins();
   Double_t xmin = axis->GetXmin();
   Double_t xmax = axis->GetBinUpEdge(nbins);
   axis->SetRange(0,0);
   axis->Set(nbins,xmin,xmax);
   Int_t  nbinsx = p->fXaxis.GetNbins();
   Int_t  nbinsy = p->fYaxis.GetNbins();
   Int_t ncells = (nbinsx+2)*(nbinsy+2);
   p->SetBinsLength(ncells);
   p->fBinEntries.Set(ncells);
   p->fSumw2.Set(ncells);
   if (p->fBinSumw2.fN)  p->fBinSumw2.Set(ncells);

   //now loop on all bins and refill
   Int_t bin,ibin,binx,biny;
   for (biny=1;biny<=nbinsy;biny++) {
      for (binx=1;binx<=nbinsx;binx++) {
         bin   = biny*(nbxold+2) + binx;
         ibin  = biny*(nbinsx+2) + binx;
         p->fArray[ibin] = hold->fArray[bin];
         p->fBinEntries.fArray[ibin] = hold->fBinEntries.fArray[bin];
         p->fSumw2.fArray[ibin] = hold->fSumw2.fArray[bin];
         if (p->fBinSumw2.fN) p->fBinSumw2.fArray[bin] = hold->fBinSumw2.fArray[bin];
      }
   }
   delete hold;
}

template <typename T>
void TProfileHelper::LabelsInflate(T* p, Option_t *ax)
{
// Double the number of bins for axis.
// Refill histogram
// This function is called by TAxis::FindBin(const char *label)

   TAxis *axis = p->GetXaxis();
   if (ax[0] == 'y' || ax[0] == 'Y') axis = p->GetYaxis();
   T *hold = (T*)p->Clone();
   hold->SetDirectory(0);

   Int_t  nbxold = p->fXaxis.GetNbins();
   Int_t  nbyold = p->fYaxis.GetNbins();
   Int_t  nbins  = axis->GetNbins();
   Double_t xmin = axis->GetXmin();
   Double_t xmax = axis->GetXmax();
   xmax = xmin + 2*(xmax-xmin);
   axis->SetRange(0,0);
   axis->Set(2*nbins,xmin,xmax);
   nbins *= 2;
   Int_t  nbinsx = p->fXaxis.GetNbins();
   Int_t  nbinsy = p->fYaxis.GetNbins();
   Int_t ncells = (nbinsx+2)*(nbinsy+2);
   p->SetBinsLength(ncells);
   p->fBinEntries.Set(ncells);
   p->fSumw2.Set(ncells);
   if (p->fBinSumw2.fN)  p->fBinSumw2.Set(ncells);

   //now loop on all bins and refill
   Int_t bin,ibin,binx,biny;
   for (biny=1;biny<=nbinsy;biny++) {
      for (binx=1;binx<=nbinsx;binx++) {
         bin   = biny*(nbxold+2) + binx;
         ibin  = biny*(nbinsx+2) + binx;
         if (binx <= nbxold && biny <= nbyold) {
            p->fArray[ibin] = hold->fArray[bin];
            p->fBinEntries.fArray[ibin] = hold->fBinEntries.fArray[bin];
            p->fSumw2.fArray[ibin] = hold->fSumw2.fArray[bin];
            if (p->fBinSumw2.fN) p->fBinSumw2.fArray[bin] = hold->fBinSumw2.fArray[bin];
         } else {
            p->fArray[ibin] = 0;
            p->fBinEntries.fArray[ibin] = 0;
            p->fSumw2.fArray[ibin] = 0;
            if (p->fBinSumw2.fN) p->fBinSumw2.fArray[bin] = 0;
         }
      }
   }
   delete hold;
}

template <typename T>
void TProfileHelper::LabelsOption(T* p, Option_t *option, Option_t *ax)
{
//  Set option(s) to draw axis with labels
//  option = "a" sort by alphabetic order
//         = ">" sort by decreasing values
//         = "<" sort by increasing values
//         = "h" draw labels horizonthal
//         = "v" draw labels vertical
//         = "u" draw labels up (end of label right adjusted)
//         = "d" draw labels down (start of label left adjusted)

   TAxis *axis = p->GetXaxis();
   if (ax[0] == 'y' || ax[0] == 'Y') axis = p->GetYaxis();
   THashList *labels = axis->GetLabels();
   if (!labels) {
      Warning("LabelsOption","Cannot sort. No labels");
      return;
   }
   TString opt = option;
   opt.ToLower();
   if (opt.Contains("h")) {
      p->fXaxis.SetBit(TAxis::kLabelsHori);
      p->fXaxis.ResetBit(TAxis::kLabelsVert);
      p->fXaxis.ResetBit(TAxis::kLabelsDown);
      p->fXaxis.ResetBit(TAxis::kLabelsUp);
   }
   if (opt.Contains("v")) {
      p->fXaxis.SetBit(TAxis::kLabelsVert);
      p->fXaxis.ResetBit(TAxis::kLabelsHori);
      p->fXaxis.ResetBit(TAxis::kLabelsDown);
      p->fXaxis.ResetBit(TAxis::kLabelsUp);
   }
   if (opt.Contains("u")) {
      p->fXaxis.SetBit(TAxis::kLabelsUp);
      p->fXaxis.ResetBit(TAxis::kLabelsVert);
      p->fXaxis.ResetBit(TAxis::kLabelsDown);
      p->fXaxis.ResetBit(TAxis::kLabelsHori);
   }
   if (opt.Contains("d")) {
      p->fXaxis.SetBit(TAxis::kLabelsDown);
      p->fXaxis.ResetBit(TAxis::kLabelsVert);
      p->fXaxis.ResetBit(TAxis::kLabelsHori);
      p->fXaxis.ResetBit(TAxis::kLabelsUp);
   }
   Int_t sort = -1;
   if (opt.Contains("a")) sort = 0;
   if (opt.Contains(">")) sort = 1;
   if (opt.Contains("<")) sort = 2;
   if (sort < 0) return;

   Int_t nx = p->fXaxis.GetNbins()+2;
   Int_t ny = p->fYaxis.GetNbins()+2;
   Int_t n = TMath::Min(axis->GetNbins(), labels->GetSize());
   Int_t *a = new Int_t[n+2];
   Int_t i,j,k,bin;
   Double_t *sumw   = new Double_t[nx*ny];
   Double_t *errors = new Double_t[nx*ny];
   Double_t *ent    = new Double_t[nx*ny];
   Double_t *entw2  = (p->fBinSumw2.fN ? new Double_t[nx*ny] : 0  ); 
   THashList *labold = new THashList(labels->GetSize(),1);
   TIter nextold(labels);
   TObject *obj;
   while ((obj=nextold())) {
      labold->Add(obj);
   }
   labels->Clear();
   if (sort > 0) {
      //---sort by values of bins
      Double_t *pcont = new Double_t[n+2];
      for (i=0;i<=n;i++) pcont[i] = 0;
      for (i=1;i<nx;i++) {
         for (j=1;j<ny;j++) {
            bin = i+nx*j;
            sumw[bin]   = p->fArray[bin];
            errors[bin] = p->fSumw2.fArray[bin];
            ent[bin]    = p->fBinEntries.fArray[bin];
            if (p->fBinSumw2.fN)  entw2[bin] = p->fBinSumw2.fArray[bin];
            if (axis == p->GetXaxis()) k = i;
            else                    k = j;
            if (p->fBinEntries.fArray[bin] != 0) pcont[k-1] += p->fArray[bin]/p->fBinEntries.fArray[bin];
         }
      }
      if (sort ==1) TMath::Sort(n,pcont,a,kTRUE);  //sort by decreasing values
      else          TMath::Sort(n,pcont,a,kFALSE); //sort by increasing values
      delete [] pcont;
      for (i=0;i<n;i++) {
         obj = labold->At(a[i]);
         labels->Add(obj);
         obj->SetUniqueID(i+1);
      }
      for (i=1;i<nx;i++) {
         for (j=1;j<ny;j++) {
            bin = i+nx*j;
            if (axis == p->GetXaxis()) {
               p->fArray[bin] = sumw[a[i-1]+1+nx*j];
               p->fSumw2.fArray[bin] = errors[a[i-1]+1+nx*j];
               p->fBinEntries.fArray[bin] = ent[a[i-1]+1+nx*j];
               if (p->fBinSumw2.fN)  p->fBinSumw2.fArray[bin] = entw2[a[i-1]+1+nx*j];
            } else {
               p->fArray[bin] = sumw[i+nx*(a[j-1]+1)];
               p->fSumw2.fArray[bin] = errors[i+nx*(a[j-1]+1)];
               p->fBinEntries.fArray[bin] = ent[i+nx*(a[j-1]+1)];
               if (p->fBinSumw2.fN)  p->fBinSumw2.fArray[bin] = entw2[i+nx*(a[j-1]+1)];
            }
         }
      }
   } else {
      //---alphabetic sort
      const UInt_t kUsed = 1<<18;
      TObject *objk=0;
      a[0] = 0;
      a[n+1] = n+1;
      for (i=1;i<=n;i++) {
         const char *label = "zzzzzzzzzzzz";
         for (j=1;j<=n;j++) {
            obj = labold->At(j-1);
            if (!obj) continue;
            if (obj->TestBit(kUsed)) continue;
            //use strcasecmp for case non-sensitive sort (may be an option)
            if (strcmp(label,obj->GetName()) < 0) continue;
            objk = obj;
            a[i] = j;
            label = obj->GetName();
         }
         if (objk) {
            objk->SetUniqueID(i);
            labels->Add(objk);
            objk->SetBit(kUsed);
         }
      }
      for (i=1;i<=n;i++) {
         obj = labels->At(i-1);
         if (!obj) continue;
         obj->ResetBit(kUsed);
      }
      for (i=0;i<nx;i++) {
         for (j=0;j<ny;j++) {
            bin = i+nx*j;
            sumw[bin]   = p->fArray[bin];
            errors[bin] = p->fSumw2.fArray[bin];
            ent[bin]    = p->fBinEntries.fArray[bin];
            if (p->fBinSumw2.fN)  entw2[bin] = p->fBinSumw2.fArray[a[bin]];
         }
      }
      for (i=0;i<nx;i++) {
         for (j=0;j<ny;j++) {
            bin = i+nx*j;
            if (axis == p->GetXaxis()) {
               p->fArray[bin] = sumw[a[i]+nx*j];
               p->fSumw2.fArray[bin] = errors[a[i]+nx*j];
               p->fBinEntries.fArray[bin] = ent[a[i]+nx*j];
               if (p->fBinSumw2.fN)  p->fBinSumw2.fArray[bin] = entw2[a[i]+nx*j];
            } else {
               p->fArray[bin] = sumw[i+nx*a[j]];
               p->fSumw2.fArray[bin] = errors[i+nx*a[j]];
               p->fBinEntries.fArray[bin] = ent[i+nx*a[j]];
               if (p->fBinSumw2.fN)  p->fBinSumw2.fArray[bin] = entw2[i+nx*a[j]];
            }
         }
      }
   }
   delete labold;
   if (a)      delete [] a;
   if (sumw)   delete [] sumw;
   if (errors) delete [] errors;
   if (ent)    delete [] ent;
   if (entw2)  delete [] entw2;
}

#endif
