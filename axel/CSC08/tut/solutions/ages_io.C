#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TPad.h"

void create_histo()
{
   // function creating a histogram using the data 
   // located in ages.txt (ages of CSC participants)
   // and saving the created histogram in a Root file

   FILE *stream;
   char line[10];
   // open the text file
   if ((stream = fopen( "ages.txt", "rt" )) == NULL ) {
      printf("Error: cannot open file ages.txt\n");
      return;
   }
   // create the 1D histogram of floats 
   // (25 bins, between 0.0 and 50.0)
   TH1F *h1 = new TH1F("ages", "ages", 25, 0, 50);
   // read each line and fill the histogram
   while (fgets(line, 10, stream)) {
      h1->Fill(atof(line));
   }
   // set fill style and color of the histogram
   h1->SetFillStyle(3001);
   h1->SetFillColor(kBlue);
   // create a file in recreate mode (to replace contents if any)
   TFile *file = new TFile("ages.root", "RECREATE");
   // write the histogram in the file
   h1->Write();
   // delete the file (the object, not the file on disk!)
   // this automatically close the physical file on disk
   delete file;
}

void fit_histo(TH1F *h1)
{
   // function fitting a 1D histogram

   // fit histogram using Root's built-in gauss function
   h1->Fit("gaus");
   // set line color of the fit
   h1->GetFunction("gaus")->SetLineColor(kRed);
   // set line width of the fit
   h1->GetFunction("gaus")->SetLineWidth(2);
   // set bar properties for the histogram
   h1->SetBarOffset(0.05);
   h1->SetBarWidth(0.9);
   // draw the histogram with "bar" style
   h1->Draw("BAR");
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

TH1F *read_histo()
{
   // function reading the 1D histogram from the Root file

   // create a file in read mode
   TFile *file = new TFile("ages.root", "READ");
   // get (read) the histogram from the file
   TH1F *h1 = (TH1F *)file->Get("ages");
   // this line makes all the difference!
   // detach histogram from the file, so the histogram
   // will not disappear when closing the file!
   h1->SetDirectory(0);
   // draw (display) the histogram
   h1->Draw();
   // delete the file. As seen before, this automatically 
   // close the physical file on disk
   delete file;
   // return pointer to the histogram
   return h1;
}

void ages_io()
{
   // main function

   // Don't display fit informations
   gStyle->SetOptFit(0);
   // Display only the following statistics informations:
   // name, number of entries, mean and rms
   gStyle->SetOptStat(1111);
   // call the function creating and saving the histogram
   create_histo();
   // call the function reading back the histogram from file
   TH1F *hist = read_histo();
   // call the function fitting the histogram
   fit_histo(hist);
}
