#include "TStyle.h"
#include "TH1F.h"
#include "TF1.h"
#include "TFormula.h"

void histogram()
{
   // Don't display fit informations
   gStyle->SetOptFit(0);
   // create 1D histogram of floats
   // note the operator new!!!!
   // the histogram is now created on the heap
   TH1F *h1 = new TH1F("h1", "Random numbers", 200, 0.0, 10.0);
   // create a formula
   TFormula *form1 = new TFormula("form1", "abs(sin(x)/x)");
   // create a 1D function using a gaussian and our formula
   TF1 *sqroot = new TF1("sqroot", "x*gaus(0) + [3]*form1", 0.0, 10.0);
   // set function parameters
   sqroot->SetParameters(10.0, 4.0, 1.0, 20.0);
   // randomly fill histogram with a distribution following
   // the previously created function
   h1->FillRandom("sqroot", 10000);
   // finally draw (display) the histogram
   h1->Draw();
}
