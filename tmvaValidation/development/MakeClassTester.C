/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Exectuable: MakeClassTester                                                    *
 *                                                                                *
 * Test suit for comparison of Reader and standalone class outputs                *
 **********************************************************************************/

const Bool_t   DEBUG = 0;
const Double_t epsilon = 1e-06; // relative allowed deviation

#include <vector>

void MakeClassTester( TString myMethodList = "" ) 
{
   cout << endl;
   cout << "==> Macro        : Start MakeClassTester" << endl;

   const Int_t Nmvas = 24;
   const char* bulkname[Nmvas] = { "Likelihood", "LikelihoodD", "LikelihoodPCA", "LikelihoodMIX", 
                                   "HMatrix", "Fisher", 
                                   "MLP", 
                                   // "BDT", "BDTD", // too large C++ file - memory problem
                                   "RuleFit", 
                                   "SVM_Gauss", "SVM_Poly", "SVM_Lin",
                                   "FDA_MT", "FDA_MC", "FDA_GA" };

   Bool_t iuse[Nmvas] = { 1, 0, 1, 0,
                          1, 1,
                          1, 
                          0, 0, 1,
                          1, 0, 0,
                          1, 0, 0 };

   // interpret input list
   if (myMethodList != "") {
      TList* mlist = TMVA::Tools::ParseFormatLine( myMethodList, " :," );
      for (Int_t imva=0; imva<Nmvas; imva++) {
         if (mlist->FindObject( bulkname[imva] )) iuse[imva] = kTRUE;
         else                                     iuse[imva] = kFALSE;
      }
      delete mlist;
   }

   //
   // create the Reader object
   //
   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );    

   // create a set of variables and declare them to the reader
   // - the variable names must corresponds in name and type to 
   // those given in the weight file(s) that you use
   std::vector<std::string> inputVars;
   inputVars.push_back( "var1+var2" );
   inputVars.push_back( "var1-var2" );
   inputVars.push_back( "var3" );
   inputVars.push_back( "var4" );

   Float_t var1, var2, var3, var4;
   reader->AddVariable( inputVars[0].c_str(), &var1 );
   reader->AddVariable( inputVars[1].c_str(), &var2 );
   reader->AddVariable( inputVars[2].c_str(), &var3 );
   reader->AddVariable( inputVars[3].c_str(), &var4 );

   // book Reader classifier and preload standalone classes
   string dir    = "weights/";
   string prefix = "TMVAnalysis";

   for (Int_t imva=0; imva<Nmvas; imva++) {
      if (iuse[imva]) {
         TString wfile = dir + prefix + "_" + bulkname[imva] + ".weights.txt";
         TString cfile = dir + prefix + "_" + bulkname[imva] + ".class.C+";

         cout << "=== Macro        : Loading weight file: " << wfile << endl;         
         cout << "=== Macro        : Loading class  file: " << cfile << endl;         

         // book the classifier through reader
         cout << TString(bulkname[imva]) + " method" << endl;
         cout << wfile << endl;
         reader->BookMVA( TString(bulkname[imva]) + " method", wfile );

         // book the classifier through standalone class
         gROOT->LoadMacro( cfile );
      }
   }
   cout << "=== Macro        : Classifier booking and class loading successfully terminated" << endl;

   // define classes
   IClassifierReader* classReader[Nmvas];
   for (Int_t i = 0; i<Nmvas; i++) = classReader[i] = 0;

   // ... and create them
   for (Int_t imva=0; imva<Nmvas; imva++) {
      if (iuse[imva]) {
         cout << "=== Macro        : Testing classifier: \"" << bulkname[imva] << "\"" << endl;
         classReader[imva]=0;
         if (bulkname[imva] == "Likelihood"   ) classReader[imva] = new ReadLikelihood   ( inputVars );
         if (bulkname[imva] == "LikelihoodD"  ) classReader[imva] = new ReadLikelihoodD  ( inputVars );
         if (bulkname[imva] == "LikelihoodPCA") classReader[imva] = new ReadLikelihoodPCA( inputVars );
         if (bulkname[imva] == "LikelihoodMIX") classReader[imva] = new ReadLikelihoodMIX( inputVars );
         if (bulkname[imva] == "Fisher"       ) classReader[imva] = new ReadFisher       ( inputVars );
         if (bulkname[imva] == "HMatrix"      ) classReader[imva] = new ReadHMatrix      ( inputVars );
         if (bulkname[imva] == "MLP"          ) classReader[imva] = new ReadMLP          ( inputVars );
         if (bulkname[imva] == "BDT"          ) classReader[imva] = new ReadBDT          ( inputVars );
         if (bulkname[imva] == "BDTD"         ) classReader[imva] = new ReadBDTD         ( inputVars );
         if (bulkname[imva] == "RuleFit"      ) classReader[imva] = new ReadRuleFit      ( inputVars );
         if (bulkname[imva] == "SVM_Gauss"    ) classReader[imva] = new ReadSVM_Gauss    ( inputVars );
         if (bulkname[imva] == "SVM_Lin"      ) classReader[imva] = new ReadSVM_Lin      ( inputVars );
         if (bulkname[imva] == "SVM_Poly"     ) classReader[imva] = new ReadSVM_Poly     ( inputVars );
         if (bulkname[imva] == "FDA_MT"       ) classReader[imva] = new ReadFDA_MT       ( inputVars );
         if (bulkname[imva] == "FDA_MC"       ) classReader[imva] = new ReadFDA_MC       ( inputVars );
         if (bulkname[imva] == "FDA_GA"       ) classReader[imva] = new ReadFDA_GA       ( inputVars );
         if(classReader[imva]==0) {
            cout << "ERROR: could not create class for " << bulkname[imva] << endl;
            exit(1);
         }
      }
   }
   cout << "=== Macro        : Class creation was successful" << endl;

   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.
   //   
   TFile *input(0);
   if (!gSystem->AccessPathName("./tmva_example.root")) {
      // first we try to find tmva_example.root in the local directory
      cout << "=== Macro        : Accessing ./tmva_example.root" << endl;
      input = TFile::Open("tmva_example.root");
   } 
   else { 
      // second we try accessing the file via the web from
      // http://root.cern.ch/files/tmva_example.root
      cout << "=== Macro        : Accessing tmva_example.root file from http://root.cern.ch/files" << endl;
      cout << "=== Macro        : For faster startup you may consider downloading it into you local directory" << endl;
      input = TFile::Open("http://root.cern.ch/files/tmva_example.root");
   }
   
   if (!input) {
      cout << "ERROR: could not open data file" << endl;
      exit(1);
   }

   // prepare the tree
   // - here the variable names have to corresponds to your tree
   // - you can use the same variables as above which is slightly faster,
   //   but of course you can use different ones and copy the values inside the event loop
   //
   TTree* theTree = (TTree*)input->Get("TreeS");
   cout << "=== Macro        : Loop over signal sample" << endl;
   Float_t userVar1, userVar2;
   theTree->SetBranchAddress( "var1", &userVar1 );
   theTree->SetBranchAddress( "var2", &userVar2 );
   theTree->SetBranchAddress( "var3", &var3 );
   theTree->SetBranchAddress( "var4", &var4 );

   // efficiency calculator for cut method
   Int_t    nSelCuts = 0, nSelCutsD = 0, nSelCutsGA = 0;
   Double_t effS     = 0.7;

   cout << "=== Macro        : Processing: " << theTree->GetEntries() << " events" << endl;
   TStopwatch sw;
   sw.Start();
   std::vector<double>* inputVec = new std::vector<double>( 4 );
   Bool_t foundProblem = kFALSE;
   for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) {

      if (ievt%1000 == 0) cout << "=== Macro        : ... processing event: " << ievt << endl;

      theTree->GetEntry(ievt);

      var1 = userVar1 + userVar2;
      var2 = userVar1 - userVar2;

      (*inputVec)[0] = var1;
      (*inputVec)[1] = var2;
      (*inputVec)[2] = var3;
      (*inputVec)[3] = var4;
      
      // loop over all booked classifiers
      for (Int_t imva=0; imva<Nmvas; imva++) {

         double clresponse = 0, rdresponse = 0;

         if (iuse[imva]) {
            
            // retrive the classifier responses            
            clresponse = classReader[imva]->GetMvaValue( *inputVec );
            rdresponse = reader->EvaluateMVA( TString(bulkname[imva]) + " method" );

            // compute deviation and print warning if too large
            Double_t deviation = rdresponse - clresponse;
            if (rdresponse != 0) deviation /= rdresponse;
            if (TMath::Abs(deviation) > epsilon ||
                TMath::IsNaN(rdresponse) || TMath::IsNaN(clresponse)) {
               foundProblem = kTRUE;
               cout << "=== Macro        : PROBLEM for classifier " << bulkname[imva]
                    << " in event " << ievt << ": the relative deviation of " 
                    << TMath::Abs(deviation) << " is larger than tolerated (" 
                    << epsilon << ")" 
                    << " -- responses (rd/cl) : " << rdresponse << " / " << clresponse 
                    << endl;
               for (Int_t iv=0;iv<inputVec->size();iv++){
                  cout << "  var[" << iv << "]="<<(*inputVec)[iv];
               }
               cout << endl;
            }
            if (DEBUG)
               cout << "=== Macro        : responses (rd/cl) : " 
                    << rdresponse << " / " << clresponse 
                    << " --> deviation: " << deviation
                    << endl;
         }
      }
   }
   
   sw.Stop();
   cout << "=== Macro        : End of event loop: "; sw.Print();
   cout << endl;
   if (foundProblem) cout << "=== Macro        : *** Problems detected *** :-(" << endl;
   else              cout << "=== Macro        : No problem found :-)" << endl;

   delete inputVec;
   delete reader;

   cout << "==> Macro        : MakeClassTester is done!" << endl << endl;
} 
