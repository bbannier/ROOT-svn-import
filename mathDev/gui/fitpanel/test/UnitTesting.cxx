#include "TFitEditor.h"
#include "TRooFitPanel.h"

#include "TApplication.h"
#include "TROOT.h"
#include "TBenchmark.h"
#include "TFile.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TF2.h"

#include "TPluginManager.h"
#include "TError.h"

#include "TGComboBox.h"
#include "TGTextEntry.h"

#include "RooWorkspace.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooDataSet.h"

#include <iostream>
#include <exception>
#include <stdexcept>
#include <cmath>
using namespace std;

#include "CommonDefs.h"

// Function that compares to doubles up to an error limit. The error
// limit can be up to 1.E-10, but it changes depending on the computer
// architecture
int equals(Double_t n1, Double_t n2, double ERRORLIMIT = 1.E-10)
{
   return fabs( n1 - n2 ) > ERRORLIMIT * fabs(n1);
}

// Selects, given a TGComboBox*, the entry whose title is name.
int SelectEntry(TGComboBox* cb, const char* name)
{
   TGTextLBEntry* findEntry = static_cast<TGTextLBEntry*>( cb->FindEntry(name) );
   cb->Select(findEntry->EntryId());

   return findEntry->EntryId();
}

// Class to make the Unit Testing. It is important than the test
// methods are inside the class as this in particular is defined as a
// friend of the TFitEditor. This way, we can access the private
// methods of TFitEditor to perform several types of tests.
class FitEditorUnitTesting 
{
private:
   // Pointer to the current (and only one) TFitEditor opened.
   TFitEditor* f;

   // These two variables are here to redirect the standard output to
   // a file.
   int old_stdout;
   FILE *out;
public:

   // Exception thrown when any of the pointers managed by the
   // FitEditorUnitTesting class are invalid
   class InvalidPointer: public std::exception 
   {
   private:
      const char* _exp;
   public:
      InvalidPointer(const char* exp): _exp(exp) {};
      const char* what() { return _exp; };
   };

   // Constructor: Receives the instance of the TFitEditor
   FitEditorUnitTesting() {
      // Redirect the stdout to a file outputUnitTesting.txt
      old_stdout = dup (fileno (stdout));
      (void) freopen ("outputUnitTesting.txt", "w", stdout);
      out = fdopen (old_stdout, "w");

      // Execute the initial script
      gROOT->ProcessLine(".x $ROOTSYS/tutorials/fit/FittingDemo.C+");
      
      // Get an instance of the TFitEditor
      TCanvas* c1 = static_cast<TCanvas*>( gROOT->FindObject("c1") );
      TH1*      h = static_cast<TH1*>    ( gROOT->FindObject("histo") );
      
      f = TFitEditor::GetInstance();
      f->Show(c1,h);
   
      if ( f == 0 )
         throw InvalidPointer("In FitEditorUnitTesting constructor");
   }

   // The destructor will close the TFitEditor and terminate the
   // application. Unfortunately, the application must be run from
   // main, otherwise, the test will make a segmentation fault while
   // trying to retrieve the TFitEditor singleton. If the user wants
   // to play a bit with the fitpanel once the tests have finised,
   // then they should comment this method.
//    ~FitEditorUnitTesting() {
//       f->DoClose();
//       gApplication->Terminate();
//    }

   // This is a generic method to make the output of all the tests
   // consistent. T is a function pointer to one of the tests
   // function. It has been implemented through templates to permit
   // more test types than the originally designed.
   // @ str : Name of the test
   // @ func : Member function pointer to the real implementation of
   // the test.
   template <typename T>
   int MakeTest(const char* str,  T func )
   {
      fprintf(stdout, "\n***** %s *****\n", str);
      int status = (this->*func)();

      fprintf(stdout, "%s..........", str);
      fprintf(out, "%s..........", str);
      if ( status == 0 ) {
         fprintf(stdout, "OK\n");
         fprintf(out, "OK\n");
      }
      else {
         fprintf(stdout, "FAILED\n");
         fprintf(out, "FAILED\n");
      }
      return status;
   }

   // This is where all the tests are called. If the user wants to add
   // new tests or avoid executing one of the existing ones, it is
   // here where they should do it.
   int UnitTesting() {
      int result = 0;

      fprintf(out, "\n**STARTING TFitEditor Unit Tests**\n\n");

      result += MakeTest("TestHistogramFit...", &FitEditorUnitTesting::TestHistogramFit);

      result += MakeTest("TestGSLFit.........", &FitEditorUnitTesting::TestGSLFit);

      result += MakeTest("TestUpdate.........", &FitEditorUnitTesting::TestUpdate);

      result += MakeTest("TestGraph..........", &FitEditorUnitTesting::TestGraph);

      result += MakeTest("TestGraphError.....", &FitEditorUnitTesting::TestGraphError);

      result += MakeTest("TestGraph2D........", &FitEditorUnitTesting::TestGraph2D);

      result += MakeTest("TestGraph2DError...", &FitEditorUnitTesting::TestGraph2DError);

      result += MakeTest("TestUpdateTree.....", &FitEditorUnitTesting::TestUpdateTree);

      result += MakeTest("TestTree1D.........", &FitEditorUnitTesting::TestTree1D); 

      result += MakeTest("TestTree2D.........", &FitEditorUnitTesting::TestTree2D);

      result += MakeTest("TestTreeND.........", &FitEditorUnitTesting::TestTreeND); 

      result += MakeTest("TestRooFitTH1......", &FitEditorUnitTesting::TestRooFitTH1); 

      result += MakeTest("TestRooFitTH2......", &FitEditorUnitTesting::TestRooFitTH2); 

//       result += MakeTest("TestRooFitSB.......", &FitEditorUnitTesting::TestRooFitSB); 

//      result += MakeTest("TestRooTree........", &FitEditorUnitTesting::TestRooTree); 

      fprintf(out, "\nRemember to also check outputUnitTesting.txt for "
              "more detailed information\n\n");

      return result;
   }

   // This is a debuggin method used to print the parameter values
   // stored in the fitpanel. This is useful when performing a fit, to
   // know against which values the test should be compare to.
   void PrintFuncPars()
   {
      static int counter = 0;
      fprintf(out, "Printing the Func Pars (%d)\n", ++counter);
      for ( unsigned int i = 0; i < f->fFuncPars.size(); ++i ) {
         fprintf(out, "%30.20f %30.20f %30.20f\n", f->fFuncPars[i][0], f->fFuncPars[i][1], f->fFuncPars[i][2]);
      }
   }

   // This function compares the parameters stored in the TFitEditor
   // with the ones passed by the test functions. Normally, if the
   // function return 0, it means all the parameters are equal up to a
   // certain limit, thus the test was succesful.
   int CompareFuncPars(std::vector<TFitEditor::FuncParamData_t>& pars)
   {
      //Uncomment to see the parameters in the current architecture
      //(this way the references can be properly set)
      //PrintFuncPars();

      int status = 0;
      for ( unsigned int i = 0; i < f->fFuncPars.size(); ++i ) {
         for ( unsigned int j = 0; j < 3; ++j) {
            int internalStatus = equals(pars[i][j], f->fFuncPars[i][j]);
//             fprintf(out, "i: %d, j: %d, ref: %30.20f, fun: %30.20f, dif: %f e: %d\n", 
// 		    i, j, pars[i][j], f->fFuncPars[i][j], TMath::Abs(pars[i][j] - f->fFuncPars[i][j]), internalStatus);
            status += internalStatus;
         }
      }

      return status;
   }

   // From here, the implementation of the different tests. The names
   // of the test should be enough to know what they are testing, as
   // these tests are mean to be as simple as possible.

   int TestHistogramFit() {
      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      f->fFuncList->Select(kFP_ALTFUNC, kTRUE);
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(6);
      pars[0][0] = -0.86471374181557547622;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 45.84337695158775005666;  pars[1][1] = pars[1][2] = 0.0;
      pars[2][0] = -13.32141785850429549498; pars[2][1] = pars[2][2] = 0.0;
      pars[3][0] = 13.80743374851078009158;  pars[3][1] = pars[3][2] = 0.0;
      pars[4][0] = 0.17230937581704022787;   pars[4][1] = pars[4][2] = 0.0;
      pars[5][0] = 0.98728095739797461228;   pars[5][1] = pars[5][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestGSLFit() {
      f->fTypeFit->Select(kFP_PREVFIT, kTRUE);
      f->fLibGSL->Toggled(kTRUE);
      f->fMinMethodList->Select(kFP_BFGS2, kTRUE);
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(6);
      pars[0][0] = -0.86471374174593529371;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 45.84337695142446733598;  pars[1][1] = pars[1][2] = 0.0;
      pars[2][0] = -13.32141785901991326568; pars[2][1] = pars[2][2] = 0.0;
      pars[3][0] = 13.80743374846000293132;  pars[3][1] = pars[3][2] = 0.0;
      pars[4][0] = 0.17230937501266346823;   pars[4][1] = pars[4][2] = 0.0;
      pars[5][0] = 0.98728095227208689799;   pars[5][1] = pars[5][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestUpdate() {
      gROOT->ProcessLine(".x $ROOTSYS/tutorials/fit/ConfidenceIntervals.C+");
      f->DoUpdate();

      return 0;
   }

   int TestGraph() {
      SelectEntry(f->fDataSet, "TGraph::GraphNoError");

      f->fLibMinuit2->Toggled(kTRUE);
      f->fMinMethodList->Select(kFP_MIGRAD, kTRUE);

      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "fpol");
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(2);
      pars[0][0] = -1.07569876898511784802;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 1.83337233651544084800;  pars[1][1] = pars[1][2] = 0.0;

      return CompareFuncPars(pars);
   }

    int TestGraphError() {
      SelectEntry(f->fDataSet, "TGraphErrors::Graph");

      f->fLibMinuit2->Toggled(kTRUE);
      f->fMinMethodList->Select(kFP_MIGRAD, kTRUE);

      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "fpol");
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(2);
      pars[0][0] = -1.07569876898508010044;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 1.83337233651530895351;  pars[1][1] = pars[1][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestGraph2D() {
      SelectEntry(f->fDataSet, "TGraph2D::Graph2DNoError");

      f->fLibMinuit2->Toggled(kTRUE);
      f->fMinMethodList->Select(kFP_MIGRAD, kTRUE);

      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "f2");
      
      // Set the parameters to the original ones in
      // ConfidenceIntervals.C. Otherwise it will be using those of
      // the last fit with fpol and will make an invalid fit.
      f->fFuncPars[0][0] = 0.5;
      f->fFuncPars[1][0] = 1.5;

      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(2);
      pars[0][0] = 0.57910401391086918643;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 1.73731204173242681499;  pars[1][1] = pars[1][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestGraph2DError() {
      SelectEntry(f->fDataSet, "TGraph2DErrors::Graph2D");

      f->fLibMinuit2->Toggled(kTRUE);
      f->fMinMethodList->Select(kFP_MIGRAD, kTRUE);

      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "f2");
      
      // Set the parameters to the original ones in
      // ConfidenceIntervals.C. Otherwise it will be using those of
      // the last fit with f2 and the fit will make no sense.
      f->fFuncPars[0][0] = 0.5;
      f->fFuncPars[1][0] = 1.5;

      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(2);
      pars[0][0] = 0.57911670684083915717;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 1.73735012087486695442;  pars[1][1] = pars[1][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestUpdateTree() {
      gROOT->ProcessLine(".x ~/tmp/fitpanel/createTree.C++");
      f->DoUpdate();
      return 0;
   }

   int TestTree1D() {
      TObject* objSelected = gROOT->FindObject("tree");
      if ( !objSelected ) 
         throw InvalidPointer("In TestUpdateTree");

      Int_t selected = kFP_NOSEL + 6;

      f->ProcessTreeInput(objSelected, selected, "x", "y>1");
      f->fTypeFit->Select(kFP_PRED1D, kTRUE);
      SelectEntry(f->fFuncList, "gausn");

      f->fFuncPars.resize(3);
      f->fFuncPars[0][0] = f->fFuncPars[0][1] = f->fFuncPars[0][2] = 1;
      f->fFuncPars[1][0] = 0;
      f->fFuncPars[2][0] = 1;

      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(3);
      pars[0][0] = 1.0;  pars[0][1] = pars[0][2] = 1.0;
      pars[1][0] = 0.57616216764976890818;  pars[1][1] = pars[1][2] = 0.0;
      pars[2][0] = 0.90739778708201823676;  pars[2][1] = pars[2][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestTree2D() {
      TObject* objSelected = gROOT->FindObject("tree");
      if ( !objSelected ) 
         throw InvalidPointer("In TestUpdateTree");

      Int_t selected = kFP_NOSEL + 6;

      f->ProcessTreeInput(objSelected, selected, "x:y", "");
      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "gaus2d");

      f->fFuncPars[0][0] = 1; f->fFuncPars[0][1] = f->fFuncPars[0][2] = 0;
      f->fFuncPars[1][0] = 1; f->fFuncPars[1][1] = f->fFuncPars[1][2] = 0;
      f->fFuncPars[2][0] = 0; f->fFuncPars[2][1] = f->fFuncPars[2][2] = 0;

      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(3);
      pars[0][0] = 1.01009862846512765699;  pars[0][1] = pars[0][2] = 0.0;
      pars[1][0] = 2.00223267618221001385;  pars[1][1] = pars[1][2] = 0.0;
      pars[2][0] = 0.49143171847344568892;  pars[2][1] = pars[2][2] = 0.0;

      return CompareFuncPars(pars);
   }

   int TestTreeND() {
      TObject* objSelected = gROOT->FindObject("tree");
      if ( !objSelected ) 
         throw InvalidPointer("In TestUpdateTree");

      Int_t selected = kFP_NOSEL + 6;

      f->ProcessTreeInput(objSelected, selected, "x:y:z:u:v:w", "");
      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "gausND");

      f->fFuncPars[ 0][0] = 1.0; f->fFuncPars[ 0][1] = f->fFuncPars[ 0][2] = 0;
      f->fFuncPars[ 1][0] = 1.0; f->fFuncPars[ 1][1] = f->fFuncPars[ 1][2] = 0;
      f->fFuncPars[ 2][0] = 0.1; f->fFuncPars[ 2][1] = f->fFuncPars[ 2][2] = 0;
      f->fFuncPars[ 3][0] = 0.0; f->fFuncPars[ 3][1] = f->fFuncPars[ 3][2] = 0;
      f->fFuncPars[ 4][0] = 2.0; f->fFuncPars[ 4][1] = f->fFuncPars[ 4][2] = 0;
      f->fFuncPars[ 5][0] = 0.0; f->fFuncPars[ 5][1] = f->fFuncPars[ 5][2] = 0;
      f->fFuncPars[ 6][0] = 3.0; f->fFuncPars[ 6][1] = f->fFuncPars[ 6][2] = 0;
      f->fFuncPars[ 7][0] = 0.0; f->fFuncPars[ 7][1] = f->fFuncPars[ 7][2] = 0;
      f->fFuncPars[ 8][0] = 4.0; f->fFuncPars[ 8][1] = f->fFuncPars[ 8][2] = 0;
      f->fFuncPars[ 9][0] = 0.0; f->fFuncPars[ 9][1] = f->fFuncPars[ 9][2] = 0;
      f->fFuncPars[10][0] = 9.0; f->fFuncPars[10][1] = f->fFuncPars[10][2] = 0;

      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(11);
      pars[ 0][0] = 1.01010131602646668902;  pars[ 0][1] = pars[ 0][2] = 0;
      pars[ 1][0] = 2.00223695401921686354;  pars[ 1][1] = pars[ 1][2] = 0;
      pars[ 2][0] = 0.49142982324998213395;  pars[ 2][1] = pars[ 2][2] = 0;
      pars[ 3][0] = 0.03058394089143435732;  pars[ 3][1] = pars[ 3][2] = 0;
      pars[ 4][0] = 2.98217419529281224655;  pars[ 4][1] = pars[ 4][2] = 0;
      pars[ 5][0] = 0.08458900612860828505;  pars[ 5][1] = pars[ 5][2] = 0;
      pars[ 6][0] = 3.97659922918498898170;  pars[ 6][1] = pars[ 6][2] = 0;
      pars[ 7][0] = -0.03584550857809707530; pars[ 7][1] = pars[ 7][2] = 0;
      pars[ 8][0] = 4.96478036967174318761;  pars[ 8][1] = pars[ 8][2] = 0;
      pars[ 9][0] = 0.89557704563234774575;  pars[ 9][1] = pars[ 9][2] = 0;
      pars[10][0] = 9.92938948024981371532;  pars[10][1] = pars[10][2] = 0;


      return CompareFuncPars(pars);
   }

   int TestRooFitTH1() {
      TH1D * h1 = new TH1D("h1-gaus","h1-gaus",100,0,100);
      TF1 * tmp = new TF1("tmp-gaus","gaus");
      tmp->SetParameters(1,50,10);
      h1->FillRandom("tmp-gaus");
      new TCanvas("canvasRooFitTH1", "Test RooFit TH1");
      h1->Draw();
      f->DoUpdate();
      SelectEntry(f->fDataSet, "TH1D::h1-gaus");
      f->fRooFitPanel->fExpRoo->SetText("Gaussian::grf(x,mu[50,0,100],sigma[10,0.1,100])");
      f->fRooFitPanel->DoGenerateRooFit();
      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "grf");
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(3);
      pars[ 0][0] = 4949.25547494941565673798;  pars[ 0][1] = pars[ 0][2] = 0;
      pars[ 1][0] = 50.17223488429569044911;    pars[ 1][1] = pars[ 1][2] = 0;
      pars[ 2][0] = 9.9872578719160127747;      pars[ 2][1] = pars[ 2][2] = 0;

      return CompareFuncPars(pars);
   }

   int TestRooFitTH2() {
      TH2D * h2 = new TH2D("h2-gaus2D","h2-gaus2D",50,0,100,50,0,100);
      TF2 * tmp = new TF2("tmp-gaus2D","xygaus");
      tmp->SetParameters(1,50,10,30,20);
      h2->FillRandom("tmp-gaus2D",100000);
      new TCanvas("canvasRooFitTH2", "Test RooFit TH2");
      h2->Draw();
      f->DoUpdate();
      SelectEntry(f->fDataSet, "TH2D::h2-gaus2D");
      f->fRooFitPanel->fExpRoo->SetText("PROD::modelrf2d(Gaussian::gx(x,mux[50,0,100],sigmax[10,0.1,100]),Gaussian::gy(y,muy[50,0,100],sigmay[10,0.1,100]))");
      f->fRooFitPanel->DoGenerateRooFit();
      f->fTypeFit->Select(kFP_UFUNC, kTRUE);
      SelectEntry(f->fFuncList, "modelrf2d");
      f->DoFit();

      std::vector<TFitEditor::FuncParamData_t> pars(5);
      pars[ 0][0] = 394457.0884414982283487916;  pars[ 0][1] = pars[ 0][2] = 0;
      pars[ 1][0] = 49.99782848579946659129;     pars[ 1][1] = pars[ 1][2] = 0;
      pars[ 2][0] = 29.97643505071316383237;     pars[ 2][1] = pars[ 2][2] = 0;
      pars[ 3][0] = 9.89280774554449848779;      pars[ 3][1] = pars[ 3][2] = 0;
      pars[ 4][0] = 19.72787235726142895942;     pars[ 4][1] = pars[ 4][2] = 0;

      return CompareFuncPars(pars);
   }

   int TestRooFitSB() {
     // This test is disabled until we figure out why the expression
     // inserted in fExpRoo is invalid. Notice that it perfectly works
     // in the macro createRooFitPdf.C

      delete f->fRooFitPanel->fWorkspace;
      f->fRooFitPanel->fWorkspace = new RooWorkspace();

      f->fRooFitPanel->fExpRoo->SetText("SUM::modelSB(S[5,0,100]*Gaussian::g(x[0,100],mu[50,0,100],sigma[10,0.1,100]),B[10,0,100]*Uniform::bpdf(x) )");
      f->fRooFitPanel->DoGenerateRooFit();


      //fprintf(out, "%d\n", f->fRooFitPanel->fWorkspace->factory("SUM::modelSB(S[5,0,100]*Gaussian::g(x[0,100],mu[50,0,100],sigma[10,0.1,100]),B[10,0,100]*Uniform::bpdf(x) )"));

      return 1;
   }

   int TestRooTree() 
     // This test is disabled until we figure out why the expression
     // inserted in fExpRoo is invalid. Notice that it perfectly works
     // in the macro createRooFitPdf.C

     TFile * file = new TFile("~/tmp/fitpanel/tree_gaus.root");
     if (!file) {
       cout << "Unable to open tree_gaus.root" << endl;
       return 1;
     }

     TTree * tree = (TTree *) file->Get("tree");
     if (!tree) { 
       cout << "Unable to retrieve tree" << endl;
       return 1;
     }

     f->DoUpdate();
     f->fRooFitPanel->fExpRoo->SetText("PROD::model(Gaussian::gx(x,mux[0,-inf,inf],sigmax[1,0.1,10]),Gaussian::gy(y,muy[0,-inf,inf],sigmay[1,0.1,10]))");


     
     return 0;
   }

};

// Runs the  basic script  and pops  out the fit  panel. Then  it will
// initialize the  FitEditorUnitTesting class and make it  run all the
// tests
int UnitTesting()
{
   FitEditorUnitTesting fUT;

   return fUT.UnitTesting();
}

// The main function. It is VERY important that it is run using the
// TApplication.
int main(int argc, char** argv)
{
   TApplication* theApp = 0;

   theApp = new TApplication("App",&argc,argv);

   int ret =  UnitTesting();

   theApp->Run();
   delete theApp;
   theApp = 0;

   return ret;
}
