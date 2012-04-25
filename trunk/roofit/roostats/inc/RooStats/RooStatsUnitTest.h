#ifndef ROOSTATS_UNIT_TEST
#define ROOSTATS_UNIT_TEST

// ROOT headers
#include "TFile.h"

// RooFit headers
#include "RooUnitTest.h"
#include "RooWorkspace.h"

// RooStats headers
#include "RooStats/ModelConfig.h"

using namespace RooStats;


class RooStatsUnitTest : public RooUnitTest {
public:
   RooStatsUnitTest(const char *name, TFile *refFile, Bool_t writeRef, Int_t verbose);
   ~RooStatsUnitTest();

private:
   RooWorkspace *fWS;
   ModelConfig *fSBModel;
   ModelConfig *fBModel;

protected:
   virtual void checkModels() {}
   virtual void checkWS() {}
   virtual void checkData() {} 
};

#endif

