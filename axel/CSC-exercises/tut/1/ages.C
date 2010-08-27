#include "TFile.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TH1F.h"
#include "TF1.h"
#include "TPad.h"

TH1F *create_histo()
{
   // function creating a histogram using the data 
   // located in ages.txt (ages of CSC participants)
   // and returning a pointer on the created histogram

   FILE *stream;
   char line[10];
   // open the text file
   if ((stream = fopen( "ages.txt", "rt" )) == NULL ) {
      printf("Error: cannot open file ages.txt\n");
      return;
   }
   // create the 1D histogram of floats 
   // (25 bins, between 0.0 and 50.0)
   TH1F *h1 = new TH1F("ages", "ages", 25, 0.0, 50.0);
   // read each line and fill the histogram
   while (fgets(line, 10, stream)) {
      h1->Fill(atof(line));
   }
   // draw (display) the histogram
   h1->Draw();
   // return the pointer on the histogram
   return h1;
}

void fit_histo(TH1F *h1)
{
   // function fitting a 1D histogram

   // fit histogram using Root's built-in gauss function
   h1->Fit("gaus");
   // create latex legend to display gauss mathematical function
   TLatex* latex = new TLatex();
   // set NDC coordinates [0,1] 
   latex->SetNDC();
   // set text color
   latex->SetTextColor(36);
   // set text size
   latex->SetTextSize(0.07);
   // draw the latex function at [0.5,0.55] NDC coordinates
   latex->DrawLatex(0.15, 0.55, "#font[12]{f(x)}#scale[0.8]{=[a]}#upoint#font[12]{e}^{ - #frac{(#scale[1.2]{#font[12]{x}}-[b])^{2}}{2#upoint[c]^{2}}}");
   // update the current pad
   gPad->Update();
}

void ages()
{
   // main function
   
   // Don't display fit informations
   gStyle->SetOptFit(0);
   // Display only the following statistics informations:
   // name, number of entries, mean and rms
   gStyle->SetOptStat(1111);
   // call the function creating the histogram
   TH1F *hist = create_histo();
   // call the function fitting the histogram
   fit_histo(hist);
}
