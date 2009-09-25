/**
   HypoTestInvertorResult class

**/

// include other header files
#include "ostream.h"

// include header file of this class 
#include "RooStats/HypoTestInvertorResult.h"
#include "RooStats/HybridResult.h"

ClassImp(RooStats::HypoTestInvertorResult)

using namespace RooStats;



HypoTestInvertorResult::HypoTestInvertorResult( const char* name,
						const char* title,
						RooRealVar* scannedVariable,
						double cl ) :
  SimpleInterval(name,title,scannedVariable,-9999,+9999)
  //TNamed( TString(name), TString(title) ),
  //fScannedVariable(scannedVariable)
{
  // constructor

//   fLowerLimit = -9999;      // default value: replace by -infinity? by 0?
//   fUpperLimit = +9999;      // default value: replace by +infinity?

  SetConfidenceLevel(cl);

  yObjects.SetOwner();
}


HypoTestInvertorResult::~HypoTestInvertorResult()
{
  // destructor
}

 
double HypoTestInvertorResult::GetXValue( int index )
{
  if ( index >= Size() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  return xValues[index];
}

double HypoTestInvertorResult::GetYValue( int index )
{
  if ( index >= Size() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  return ((HybridResult*)yObjects.At(index))->CLs();
}


void HypoTestInvertorResult::CalculateLimits()
{ 
  // find the 2 objects the closer to the limit and make a linear extrapolation to the target

  double cl = 1-ConfidenceLevel();

  if (Size()<2) {
    std::cout << "not enough points to get the inverted interval\n";
  }

  double v1 = fabs(GetYValue(0)-cl);
  int i1 = 0;
  double v2 = fabs(GetYValue(1)-cl);
  int i2 = 1;

  if (Size()>2)
    for (int i=2; i<Size(); i++) {
      double vt = fabs(GetYValue(i)-cl);
      std::cout << i << "  " << vt << endl;
      if ( vt<v1 || vt<v2 ) {
	if ( v1<v2 ) {
	  v2 = vt;
	  i2 = i;
	} else {
	  v1 = vt;
	  i1 = i;
	}
      }
    }

  fLowerLimit = ((RooRealVar*)fParameters->first())->getMin();

  fUpperLimit = GetXValue(i1)+(cl-GetYValue(i1))*(GetXValue(i2)-GetXValue(i1))/(GetYValue(i2)-GetYValue(i1)); // MAYBE TOO MANY GETYVALUE CALLS!

  return;
}
