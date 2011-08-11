#include "TFileCollection.h"
#include "TProof.h"
#include "TString.h"

void makeDataSet(const char *tag)
{

   // Parse test tag to get relevant config parameters
   TString fnlist = TString::Format("filelists/%s.txt", tag);

   if (gSystem->AccessPathName(fnlist)) return;
   
   // Create collection
   TFileCollection *fc = new TFileCollection("","",fnlist.Data());

   // Start Proof
   TProof *p = TProof::Open("lite://");
   if (p && p->IsValid()) {
      p->RegisterDataSet(tag, fc, "OV");
   }
}


void makeDataSets()
{
   makeDataSet("data_20_c0_s99");
   makeDataSet("data_200_c0_s99");
   makeDataSet("data_20_c0_s0");
   makeDataSet("data_200_c0_s0");
   makeDataSet("data_20_c1_s99");
   makeDataSet("data_200_c1_s99");
   makeDataSet("data_20_c1_s0");
   makeDataSet("data_200_c1_s0");
}
