#include <stdexcept>
#include <iostream>

#include "EuropeMapDemo.h"

#include "TRandom.h"
#include "TH2Poly.h"
#include "TFile.h"
#include "TMath.h"
#include "TKey.h"


#include "EuropeMapDemo.h"
#include "PadProxy.h"

namespace ROOT_iOS {
namespace Demos {

//______________________________________________________________________________
EuropeMapDemo::EuropeMapDemo()
{
   std::auto_ptr<TFile> inputFile(TFile::Open("http://root.cern.ch/files/europe.root", "read"));
   if (!inputFile.get())
      throw std::runtime_error("could not open network file");

   // Real surfaces taken from Wikipedia.
   const UInt_t nx = 36;
   const UInt_t nPoints = 500000;
   // see http://en.wikipedia.org/wiki/Area_and_population_of_European_countries
   const Float_t surfaces[nx] = { 547030,        505580,   449964,      357021,        338145, 
                                  324220,        312685,   301230,      255438,        244820, 
                                  237500,        207600,   131940,      127711,        110910, 
                                  103000,         93030,    89242,       83870,         70280, 
                                  65200,         64589,    45226,       43094,         41526, 
                                  41290,         33843,    30528,       28748,          9250, 
                                  2586,           468,      316,         160,            61,
                                  2};

   const Double_t lon1 = -25.;
   const Double_t lon2 = 35.;
   const Double_t lat1 = 34.;
   const Double_t lat2 = 72.;

   fPoly.reset(new TH2Poly("Europe", "Europe (bin contents are normalized to the surfaces in km^{2})", lon1, lon2, lat1, lat2));
   fPoly->GetXaxis()->SetNdivisions(520);
   fPoly->GetXaxis()->SetTitle("longitude");
   fPoly->GetYaxis()->SetTitle("latitude");
   fPoly->SetContour(100);

   TIter nextKey(gDirectory->GetListOfKeys());
   while (TKey *key = (TKey *)nextKey()) {
      TObject *obj = key->ReadObj();
      if (obj->InheritsFrom("TMultiGraph"))
         fPoly->AddBin(obj);
   }

   fPoly->ChangePartition(100, 100);

   // Fill TH2Poly according to a Mercator projection.
   const Double_t pi4 = TMath::Pi() / 4;
   const Double_t alpha = TMath::Pi() / 360;
   
   TRandom r;
   
   for (UInt_t i = 0; i < nPoints; ++i) {
      const Double_t longitude = r.Uniform(lon1, lon2);
      const Double_t latitude = r.Uniform(lat1, lat2);
      const Double_t x = longitude;
      const Double_t y = 38 * TMath::Log(TMath::Tan(pi4 + alpha * latitude));
      fPoly->Fill(x,y);
   }


   const Double_t maximum = fPoly->GetMaximum();

   // Normalize the TH2Poly bin contents to the real surfaces.
   const Double_t scale = surfaces[0] / maximum;
   for (UInt_t i = 0; i < fPoly->GetNumberOfBins(); ++i)
      fPoly->SetBinContent(i + 1, scale * fPoly->GetBinContent(i+1));
}

//______________________________________________________________________________
EuropeMapDemo::~EuropeMapDemo()
{
   //For auto_ptr dtor only.
}

//______________________________________________________________________________
void EuropeMapDemo::PresentDemo()
{
   fPoly->Draw("COLZ");
}


}
}
