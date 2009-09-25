/**
   HypoTestInvertorPlot class

**/

// include other header files
#include "ostream.h"
#include "RooStats/HybridResult.h"

// include header file of this class 
#include "RooStats/HypoTestInvertorPlot.h"

ClassImp(RooStats::HypoTestInvertorPlot)

using namespace RooStats;



HypoTestInvertorPlot::HypoTestInvertorPlot( const char* name,
					    const char* title,
					    HypoTestInvertorResult* results ) :
  TNamed( TString(name), TString(title) ),
  fResults(results)
{
  // constructor
}


TGraph* HypoTestInvertorPlot::MakePlot()
{
  const int nEntries = fResults->Size();

  Double_t xArray[nEntries];
  Double_t yArray[nEntries];
  for (int i=0; i<nEntries; i++) {
    xArray[i] = fResults->GetXValue(i);
    yArray[i] = fResults->GetYValue(i);
  }
  
  // sort the arrays based on the x values (using Gnome-sort algorithm)
  if (nEntries>1) {
    int i=1;
    int j=2;
    while ( i<nEntries ) {
      if ( i==0 || xArray[i-1] <= xArray[i] ) {
	i=j;
	j++;
      } else {
	double tmp = xArray[i-1];
	xArray[i-1] = xArray[i];
	xArray[i] = tmp;
	tmp = yArray[i-1];
	yArray[i-1] = yArray[i];
	yArray[i] = tmp;
	i--;
      }
    }
  }

// // points must be sorted before using a TSpline or the binary search
// 752 	  	       std::vector<Double_t> xsort(fNpoints);
// 753 	  	       std::vector<Double_t> ysort(fNpoints);
// 754 	  	       std::vector<Int_t> indxsort(fNpoints);
// 755 	  	       TMath::Sort(fNpoints, fX, &indxsort[0], false );
// 756 	  	       for (Int_t i = 0; i < fNpoints; ++i) {
// 757 	  	          xsort[i] = fX[ indxsort[i] ];
// 758 	  	          ysort[i] = fY[ indxsort[i] ];
// 759 	  	       }


  TGraph* graph = new TGraph(nEntries,xArray,yArray);
  return graph;
}

HypoTestInvertorPlot::~HypoTestInvertorPlot()
{
  // destructor
}
