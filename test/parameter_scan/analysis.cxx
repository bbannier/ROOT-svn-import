
// This program books a single TMVA classifier (specified by the
// command line options) and writes TMVA's ROOT output file as well as
// a text file with the classifier and factory options and the
// classifier output.
//
// Note, that the path of the TMVA library must be specified in the
// makefile before compiling this program.
//
// For a list of program parameters see 'main()' below or 'scan.py'.

// ROOT
#include "TCut.h"
#include "TFile.h"
#include "TRFIOFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "TH1.h"
#include "TStopwatch.h"
#include "TRandom.h"
#include "TRandom3.h"
#include "TROOT.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TKey.h"

// TMVA
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/Config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

typedef std::vector<std::pair<TString,TString> > TData;

using namespace std;

//=========================================================
// Program parameters
//=========================================================

// file to write the numerical results to (text file)
TString  txtoutfilename = "./results.txt";
// file to write th TMVA output to (root file)
TString  rootoutfilename = "./results.root";

// method optionstring
TString  method_optstr  = "!H:!V:";
// factory optionstring
TString  factory_optstr = "!V:nTrain_signal=10000:nTrain_background=10000:nTest_signal=10000:nTest_background=10000:SplitMode=Random:NormMode=EqualNumEvents";

// filename of event sample
TString  sample = "";

// names of the signal and background trees
TString  TreeSig = "TreeS";
TString  TreeBkg = "TreeB";

// list of variables (comma separated, without spaces)
TString  variables_list = "var1";

// method name as specified by the REGISTER_METHOD() macro
TString  method_name = "PDEFoam";

//=========================================================

string DoubleToString(Double_t value)
{
   // convert Double_t to string
   ostringstream o;
   if (!(o << value))
      return "";
   return o.str();
}

// ===========================================================

string IntToString(Int_t value)
{
   // convert Int_t to string
   ostringstream o;
   if (!(o << value))
      return "";
   return o.str();
}

// ===========================================================

Int_t Exe(TString cmd)
{
   // execute a shell command
   cout << ">>> Checking if processor is available...";
   if (system(NULL)) {
      cout << "Ok" << endl;
      cout << ">>> Executing command: " << cmd << endl;
      return system( cmd.Data() );
   }
   else {
      cout << "Error: processor is not available!" << endl;
      return 1;
   }
}

// ===========================================================

TData GetDataFromOptStr(TString optstr)
{
   // extract data from option string:
   //
   //   optstr = "!V:!H:opt1=T:opt2=0.3"
   //
   // and return it in a set of touples:
   //
   //   [ (opt1,T) , (opt2,0.3) ]

   TData touple;

   cout << ">>> parse option string: " << optstr << endl;

   // split option string at :
   string opt;
   stringstream stream(optstr.Data());
   while( getline(stream, opt, ':') ) {
      TString v = TString(opt).Remove(TString::kBoth,' ').Remove(TString::kBoth,':');
      if (!v.IsNull() && v != "V" && v != "H" && v != "!V" && v != "!H") {
         // split string at =
         string str;
         stringstream stream2(v.Data());
         std::vector<TString> substr;
         while( getline(stream2, str, '=') )
            substr.push_back(TString(str));
         if (substr.size() != 2) {
            cout << "Error: something is wrong with option string" << endl;
            continue;
         }
         touple.push_back(std::make_pair<TString,TString>(substr.at(0),substr.at(1)));
         cout << ">>>    found parameter \'" << substr.at(0) << "\' "
              << "with value \'" << substr.at(1) << "\'" << endl;
      }
   }

   return touple;
}

// ===========================================================

void analysis()
{
   // This function books the classifier and writes the output files
   // to the specified locations

   TStopwatch timer_benchmark, timer_train, timer_test;

   timer_train.Start();
   cout << "Start TMVAClassification" << endl
        << "========================" << endl
        << endl;

   // Create ROOT output file
   TFile *ROOTOutputFile = TFile::Open( rootoutfilename, "RECREATE" );
   if (ROOTOutputFile==0 || ROOTOutputFile->IsZombie()) {
      cout << "Error: can not create file: " << rootoutfilename << endl;
      exit(1);
   }
   // Create TMVA factory
   TMVA::Factory *factory = new TMVA::Factory( TString("TMVAClassification"),
                                               ROOTOutputFile,
                                               TString("!V:Color=F:DrawProgressBar=F:AnalysisType=Classification") );

   // directory to store the weight files in
   TString weigthFileDir(rootoutfilename);
   // remove file name from weigthFileDir --> only path survives
   while (weigthFileDir[weigthFileDir.Length()-1] != '/')
      weigthFileDir.Remove(weigthFileDir.Length()-1);
   (TMVA::gConfig().GetIONames()).fWeightFileDir = weigthFileDir;

   // accessing ROOT sample
   TFile *input(0);
   if (!gSystem->AccessPathName( sample )) {
      cout << "--- TMVAClassification : accessing " << sample << endl;
      input = TFile::Open( sample );
      if (!input) {
         cout << "ERROR: could not open data file: " << sample << endl;
         exit(3);
      }
   } else {
      cout << "ERROR: can not find file: " << sample << endl;
      exit(2);
   }

   // load the signal and background trees
   TTree *signal     = (TTree*)input->Get(TreeSig);
   TTree *background = (TTree*)input->Get(TreeBkg);

   if (!signal) {
      cout << "Error: could not find signal tree: " << TreeSig << endl;
      exit(4);
   }
   if (!background) {
      cout << "Error: could not find background tree: " << TreeBkg << endl;
      exit(4);
   }

   // cd into output root file
   gROOT->cd( rootoutfilename + TString(":/") );
   // add signal and background trees
   factory->AddSignalTree    ( signal,     1.0 );
   factory->AddBackgroundTree( background, 1.0 );

   cout << ">>> Adding variables to factory:" << endl;
   string var;
   stringstream stream(variables_list.Data());
   while( getline(stream, var, ',') ) {
      TString v = TString(var).Remove(TString::kBoth,' ');
      if (v != "") {
         cout << ">>>   add variable: " << v << endl;
         factory->AddVariable(v, 'F');
      }
   }

   // prepare factory and trees
   factory->PrepareTrainingAndTestTree( "", factory_optstr );

   // book method
   cout << ">>> Booking method: " << method_name << endl;
   factory->BookMethod( TMVA::Types::Instance().GetMethodType(method_name),
                        method_name, method_optstr );

   // Train MVAs using the set of training events
   cout << ">>> Training..." << endl;
   factory->TrainAllMethods();

   // benchmark
   timer_train.Stop();
   Double_t traintime=timer_train.CpuTime();
   printf("CPU time used for training: %4.2f s\n", traintime);
   timer_test.Start();
   // testing
   cout << ">>> Testing..." << endl;
   factory->TestAllMethods();
   // ----- Evaluate and compare performance of all configured MVAs
   cout << ">>> Evaluating..." << endl;
   factory->EvaluateAllMethods();    // error!
   timer_test.Stop();
   Double_t testtime=timer_test.CpuTime();
   printf("CPU time used for testing and evaluating: %4.2f s\n", testtime);

   // final output
   cout << ">>> TMVAClassification is done!" << endl;

   // Clean up
   delete factory;  factory = 0;

   // --------------------------------------------------------------

   // ------- get performance
   // test option string for boosting
   UInt_t Boost_Num = 0;
   TSubString boost_str = method_optstr(TRegexp("Boost_Num=[0-9]+"));
   if (!boost_str.IsNull()) {
      TString Boost_Num_str(boost_str.Data());
      Boost_Num = TString(Boost_Num_str(TRegexp("[0-9]+")).Data()).Atoi();
   }
   ROOTOutputFile->cd( (Boost_Num > 0 ? "/Method_Boost/"
                        : "/Method_"+method_name+"/")
                       + method_name);
   TH1F *roc_hist = (TH1F*)gROOT->FindObject("MVA_"+method_name+"_rejBvsS");
   Float_t ROCintegral=0;
   if (roc_hist) {
      Float_t integral = roc_hist->Integral();
      Int_t nbins = roc_hist->GetNbinsX();
      if (nbins>0)
         ROCintegral=integral/nbins;
   }
   cout << ">>> Integral under ROC curve: " << ROCintegral << endl;

   // -------- get boost data

   // find boosting histograms (name startes with 'Booster_')
   vector<TString> hname;
   vector<TH1F*> hboost;
   TIter next(gDirectory->GetListOfKeys());
   TKey *key(0);
   while ((key = (TKey*)next())) {
      if (!TString(key->GetName()).BeginsWith("Booster_"))
         continue;
      hname.push_back(TString(key->GetName()));
      hboost.push_back( (TH1F*) gROOT->FindObject(hname.back()) );
      cout << ">>> Boosting histogram found: " << hname.back() << endl;
   }

   // get number of boosts from the boost histograms.  this number
   // might be smaller than the requested number of boosts
   // 'Boost_Num' because the algorithm may have stopped due to a
   // too bad misclassification rate.
   for (UInt_t ihist=0; ihist<hboost.size(); ihist++) {
      if (hboost.at(ihist) != 0) {
         Boost_Num = hboost.at(ihist)->GetNbinsX();
         break;
      }
   }
   cout << ">>> Classifier boosted " << Boost_Num << " times" << endl;

   // check if text data file already exists
   Bool_t TextResultsFileExists = kTRUE;
   if (access(txtoutfilename, F_OK)){
      cout << ">>> file: " << txtoutfilename << " will be created" << endl;
      TextResultsFileExists = kFALSE;
   }

   // write text data to file
   ofstream txtfile(txtoutfilename, ios::out | ios::app);
   if (txtfile.is_open()) {

      // if file does not exist, the first line shall contain the
      // description of the data
      TData data(GetDataFromOptStr("MethodName="+method_name
                                   +':'+factory_optstr
                                   +':'+method_optstr
                                   +':'+"ROC_integral="+DoubleToString(ROCintegral)
                                   +":ROOTOutput="+rootoutfilename));
      if (!TextResultsFileExists) {
         txtfile << "# ";
         TData::iterator idata;
         Int_t EntryCounter = 1; // numerate data entries
         for (idata=data.begin(); idata!=data.end(); idata++) {
            txtfile << "[" << IntToString(EntryCounter) << "] "
                    << idata->first << " | ";
            EntryCounter++;
         }

         if (Boost_Num > 0) {
            // boosting was activated --> extract additional data from
            // the boosting histograms and append it to the line
            txtfile << "[" << IntToString(EntryCounter) << "] "
                    << "boost index" << " | ";
            EntryCounter++;
            for (UInt_t ihist=0; ihist<hname.size(); ihist++){
               txtfile << "[" << IntToString(EntryCounter) << "] "
                       << hname.at(ihist) << " | ";
               EntryCounter++;
            }
         } // Boost_Num > 0

         txtfile << endl;
      } // if (!TextResultsFileExists)

      // loop over all boosts and write 'Boost_Num' lines, which
      // contain this data (boost index == 0 corresponds to no boosting)
      for (UInt_t iboost=0; iboost<(Boost_Num==0?1:Boost_Num); iboost++){
         // print all results to text file
         TData::iterator idata;
         for (idata=data.begin(); idata!=data.end(); idata++)
            txtfile << idata->second << " ";

         if (Boost_Num > 0) {
            // boosting was activated --> extract additional data from
            // the boosting histograms and append it to the line
            txtfile << iboost << " "; // number of boost
            for (UInt_t ihist=0; ihist<hboost.size(); ihist++){
               if (hboost.at(ihist) == 0) {
                  txtfile << 0.0 << " ";
                  continue;
               }
               // sanity check for correct number of bins
               if (iboost+1 > (UInt_t) hboost.at(ihist)->GetNbinsX()) {
                  cout << "Warning: boost number > histogram bin number: "
                       << iboost+1 << " > " << hboost.at(ihist)->GetNbinsX()
                       << endl;
                  txtfile << 0.0 << " ";
                  continue;
               }
               // write histogram bin content
               txtfile << hboost.at(ihist)->GetBinContent(iboost+1) << " ";
            } // loop over boost histograms
         } // Boost_Num > 0
         txtfile << endl;
      } // loop over number of boosts

      cout << ">>> Wrote text data to: " << txtoutfilename << endl;
   } // file is open
   else {
      cerr << ">>> Could not open txt file for writing results: "
           << txtoutfilename << endl;
      ROOTOutputFile->Close();
      return;
   }
   txtfile.close();
   ROOTOutputFile->Close();

   //    // copy foams and data to specified location (allways accept
   //    // certificates)
   //    Exe("scp -o StrictHostKeyChecking=no " + outfileName  +" "+data_dir);
   //    Exe("scp -o StrictHostKeyChecking=no " + foam_filename+" "+data_dir);
}

// ===========================================================

int main(int argc, char **argv)
{

   Int_t opt=0;

   while((opt=getopt(argc,argv,"b:e:f:m:o:r:s:t:v:"))!=EOF) {
      switch(opt) {
      case 'b':
         TreeBkg = optarg;
         break;
      case 'e':
         sample = optarg;
         break;
      case 'f':
         factory_optstr = optarg;
         break;
      case 'm':
         method_name = optarg;
         break;
      case 'o':
         method_optstr = optarg;
         break;
      case 'r':
         rootoutfilename = optarg;
         break;
      case 's':
         TreeSig = optarg;
         break;
      case 't':
         txtoutfilename = optarg;
         break;
      case 'v':
         variables_list = optarg;
         break;
      default:
         cout << "usage: analysis "
              << " [-b background tree name]"
              << " [-e event sample]"
              << " [-f factory option string]"
              << " [-m method name]"
              << " [-o method option string]"
              << " [-r root output file name]"
              << " [-s signal tree name]"
              << " [-t text output file name]"
              << " [-v comma separated list of variables]"
              << endl;
         exit(0);
      }
   }

   // debug output (all used patameters)
   cout << "------------------" << endl;
   cout << "Program parameters" << endl;
   cout << "------------------" << endl;
   cout << "[-b background tree name]: " << TreeBkg << endl;
   cout << "[-e event sample]: " << sample << endl;
   cout << "[-f factory option string]: " << factory_optstr << endl;
   cout << "[-m method name]: " << method_name << endl;
   cout << "[-o method option string]: " << method_optstr << endl;
   cout << "[-r root output file name]: " << rootoutfilename << endl;
   cout << "[-s signal tree name]: " << TreeSig << endl;
   cout << "[-t text output file name]: " << txtoutfilename << endl;
   cout << "[-v comma separated list of variables]: " << variables_list << endl;
   cout << "------------------" << endl;

   // start analysis
   analysis();
}
