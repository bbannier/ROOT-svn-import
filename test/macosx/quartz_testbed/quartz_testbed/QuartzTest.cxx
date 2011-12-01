#include "QuartzTest.h"

#include "TH1.h"

void do_test()
{
   TH1F * hist = new TH1F("a", "b", 100, -1., 1.);
   hist->FillRandom("gaus");
}