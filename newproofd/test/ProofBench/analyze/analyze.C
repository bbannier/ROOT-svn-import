// The perfstats files can be analyzed with selectors, similar to procRates.C
// The 'analyze' function can be used for that purpose.
// e.g. .x analyze.C+("path", "procRate.C+")
// The procRates.C selector draws the processing rate in the time of a query.

#include "TChain.h"

void analyze(const char* fileName,
             const char* sel,
             const char* tree = "PROOF_PerfStats")
{
   TChain *chain = new TChain(tree);
   chain->Add(fileName);
   chain->Process(sel);
}
