#include "TRooFitPanel.h"

#include "TFitEditor.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TGProgressBar.h"
#include "TGComboBox.h"
#include "TGMsgBox.h"

#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph2D.h"
#include "THStack.h"
#include "TMultiGraph.h"

#include <vector>
#include <map>

#include "CommonDefs.h"
#include "RConfigure.h"

#ifdef R__HAS_ROOFIT
#include "RooWorkspace.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#else
struct RooWorkspace { };
#endif

//______________________________________________________________________________
TRooFitPanel::TRooFitPanel(const TGWindow* p)
  :TGVerticalFrame(p)
{
   TGHorizontalFrame *tmpFrame = new TGHorizontalFrame(this);
   TGLabel* tmpLabel = new TGLabel(tmpFrame, "Name:");
   tmpFrame->AddFrame(tmpLabel,new TGLayoutHints(kLHintsNormal, 0, 0, 2, 0));
   TGComboBox* fNameRoo = new TGComboBox(tmpFrame, kFP_NAMEROO);
   fNameRoo->Resize(190, 20);
   tmpFrame->AddFrame(fNameRoo, new TGLayoutHints(kLHintsExpandX, 23, 0, 0, 0));
   this->AddFrame(tmpFrame, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 0));

   tmpFrame = new TGHorizontalFrame(this);
   tmpLabel = new TGLabel(tmpFrame, "Formula:");
   tmpFrame->AddFrame(tmpLabel,new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));
   fExpRoo = new TGTextEntry(tmpFrame, new TGTextBuffer(0), kFP_EXPROO);
   fExpRoo->SetToolTipText("Enter a RooFit formula expression");
   fExpRoo->Resize(190,fExpRoo->GetDefaultHeight());
   tmpFrame->AddFrame(fExpRoo,new TGLayoutHints(kLHintsExpandX, 10, 0, 2, 2));
   this->AddFrame(tmpFrame, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 0));

   tmpFrame = new TGHorizontalFrame(this);
   fProgRoo = new TGHProgressBar(tmpFrame,TGProgressBar::kStandard,300 );
   fProgRoo->SetFillType(TGProgressBar::kBlockFill);
   fProgRoo->Resize(190,20);
   //fProgRoo->Increment(30);
   tmpFrame->AddFrame(fProgRoo,new TGLayoutHints(kLHintsExpandX, 10, 0, 2, 2));
   this->AddFrame(tmpFrame, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 0));

   tmpFrame = new TGHorizontalFrame(this);
   fGenRoo = new TGTextButton(tmpFrame, "&Generate", kFP_GENROO);
   fGenRoo->Resize(200,20);
   fGenRoo->SetToolTipText("Generates a TF1 from the RooFit expression");
   tmpFrame->AddFrame(fGenRoo,new TGLayoutHints(kLHintsRight | kLHintsCenterY | kLHintsExpandX));
   this->AddFrame(tmpFrame, new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 250, 5, 10, 0));

   fWorkspace = new RooWorkspace(); 
}

TRooFitPanel::~TRooFitPanel()
{
   delete fWorkspace;
   fWorkspace = 0;
}

//______________________________________________________________________________
TF1 * TRooFitPanel::CreateRooFitPdf(const char * expr, bool norm) { 

#ifdef R__HAS_ROOFIT

   RooAbsArg * arg = fWorkspace->factory(expr);
   if (!arg) { 
      std::cerr << "Error: creating RooFit model - invalid expression" << std::endl;
      return 0; 
   }
   TString modelName = arg->GetName();
   RooAbsReal * pdf = 0; 

   // add normalization term as an extra parmeters
   TString fname = TString("N_") + modelName; 
   if (!norm) { 
      // construct normalize functions 
      //TString fullexpr = TString("prod::") + fname + TString("(N[1,0,1e10],") + modelName + TString(")");
      TString fullexpr = Form("prod::fN_%s(N_%s[1,0,1e10],%s)",modelName.Data(), modelName.Data(), modelName.Data());
      RooAbsArg * tmp = fWorkspace->factory(fullexpr);
      pdf = dynamic_cast<RooAbsReal *>(tmp);
      if (!pdf) { 
         std::cerr << "Error creating Unnormalized RooFit funciton : " << fullexpr << std::endl;
         return 0; 
      }
   }



   // assume observables are x (or y, z ) for multi-dimensional functions

   RooRealVar * x = fWorkspace->var("x");
   if (!x) { 
      std::cerr << "Error: variable x not present in pdf" << std::endl;
      return 0; 
   }
   RooArgSet obs(*x);

   RooRealVar * y = fWorkspace->var("y");
   if (y) obs.add(*y);

   RooRealVar * z = fWorkspace->var("z");
   if (z) obs.add(*z);

   // in case of multi dimension needs to add also y and z
   RooArgSet * params = pdf->getParameters(obs); 

   if (!params) {
      std::cerr << "Error: no parameters present in pdf" << std::endl;
      return 0; 
   }
   params->Print();
   // adding *x as thirs parameter will ensure x is normalized with respect to x 
   TF1 * f1 = pdf->asTF(obs,RooArgList(*params),obs);
   f1->SetName(modelName);
  

   f1->SetTitle(expr);
   return f1; 

#else
   std::cerr << "Error: ROOFIT not enable - cannot create pdf " 
             << expr << std::endl;
   if (norm) std::cerr << std::endl;
   return 0; 
#endif   
}

//______________________________________________________________________________
void TRooFitPanel::DoGenerateRooFit()
{
#ifdef R__HAS_ROOFIT

   TFitEditor* fe = TFitEditor::GetInstance();

   TH1* histo = 0;
   switch (fe->fType) {
      case kObjectHisto:
         histo = static_cast<TH1*>(fe->fFitObject);
         break;
      case kObjectGraph:
         histo = ((TGraph*)fe->fFitObject)->GetHistogram();
         break;
      case kObjectGraph2D:
         histo = ((TGraph2D*)fe->fFitObject)->GetHistogram("empty");
         break;
      case kObjectHStack:
         histo = (TH1 *)((THStack *)fe->fFitObject)->GetHists()->First();
         break;
      case kObjectMultiGraph:
         histo = ((TMultiGraph*)fe->fFitObject)->GetHistogram();
         break;

      case kObjectTree:
         new TGMsgBox(fClient->GetRoot(), GetMainFrame(),
                      "Error...", "RooFit not yet supported in the FitPanel with TTree objects!",
                      kMBIconStop,kMBOk, 0);
         return;
   };

   fWorkspace->factory(Form("x[%f,%f]",histo->GetXaxis()->GetXmin(),histo->GetXaxis()->GetXmax())) ;   
   if (histo->GetDimension() > 1)
      fWorkspace->factory(Form("y[%f,%f]",histo->GetYaxis()->GetXmin(),histo->GetYaxis()->GetXmax())) ;   
   if (histo->GetDimension() > 2)
      fWorkspace->factory(Form("z[%f,%f]",histo->GetZaxis()->GetXmin(),histo->GetZaxis()->GetXmax())) ;   
   
   CreateRooFitPdf(fExpRoo->GetText());

#else
   new TGMsgBox(fClient->GetRoot(), GetMainFrame(),
                "Error...", "RooFit not yet supported in the FitPanel with TTree objects!",
                kMBIconStop,kMBOk, 0);
   return;
#endif
   return;
}

//______________________________________________________________________________
void TRooFitPanel::ConnectSlots()
{
   fGenRoo->Connect("Clicked()", "TRooFitPanel", this, "DoGenerateRooFit()");
}

//______________________________________________________________________________
void TRooFitPanel::DisconnectSlots()
{
   fGenRoo->Disconnect("Clicked()");
}
