#include <iostream> // Stream declarations

#include "TMVA_Factory.h"
#include "TMVA_MethodTMlpANN.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TMVA_Tools.h"
#include "TMVA_PDF.h"
#include "TMVA_Utils.h"


using namespace std;
using namespace TMVA_Utils;

int main(int argc, char** argv) {

  //define the config parameters:
  TMVA_Config *config=TMVA_Config::Instance();

  config->defineParameter(
    new TMVA_ConfigLineBool("useCuts", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useBDT", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("usePDERS", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useLikelihood", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useLikelihoodD", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useFisher", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useHMatrix", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useCFMlpANN", kFALSE, "%d",    "0: No, 1: Yes"));
  config->defineParameter(
    new TMVA_ConfigLineBool("useTMlpANN", kFALSE, "%d",    "0: No, 1: Yes"));

  config->defineParameter(
    new TMVA_ConfigLineString("DataSample","Toy","%s","Data Sample used"));

  config->defineParameter(
    new TMVA_ConfigLineIntValue("NTrainingEvents", 10000, "%d",    "# of training events used"));


  config->defineParameter(
    new TMVA_ConfigLineIntValue("BDT::NTrees", 20, "%d", "# of training events used"));
  config->defineParameter(
    new TMVA_ConfigLineIntValue("BDT::NodeMinEvents", 10, "%d", "min events in node"));
  config->defineParameter(
    new TMVA_ConfigLineDoubleValue("BDT::NodeMinSepGain",0.0, "%d", "min node separation gain"));

  config->defineParameter(
    new TMVA_ConfigLineIntValue("BDT::NCuts", 100, "%d", "# of cuts used in node training"));

  config->defineParameter(
    new TMVA_ConfigLineString("BDT::Boost","AdaBoost", "%s", "which boost type"));
  config->defineParameter(
    new TMVA_ConfigLineString("BDT::Separation","GiniIndex", "%s", "which sepearation method"));

  
  
  cout<<" read config file:"<<endl;
  if (argc>1) {
    std::string configFile((argv[1]));
    cout << "--- Reading config file from " << configFile <<endl;
    config->readParameterFile(configFile);
  }

  cout << endl;
  cout << "==> start TMVAnalysis" << endl;

  // the root output file
  TFile* target = TFile::Open( "TMVAnalysis.root", "RECREATE" );

  //
  // create the vactory object and claim which variance tool you 
  // would like to use:
  //
  TMVA_Factory *factory = new TMVA_Factory( "mvatest", target, "V" ) ;

  // this is the variable vector, defining what's used in the TMVA
  vector<TString>* inputVars = new vector<TString>;
  
  // load input trees
  //
  if (*((std::string*)config->value("DataSample")) == "GammaJet") {

    cout << "==> perform 'GammaJet' analysis" << endl;

    // ascii files
    TString path = "data/";
    TString datFileS = path + "tree_true_head.dat";
    TString datFileB = path + "tree_fake_head.dat";

    // create trees from these files
    if (!factory->SetInputTrees( datFileS, datFileB )) exit(1);

    // inputVars->push_back("Row");
    // inputVars->push_back("eta");
    inputVars->push_back("hadleak");
    inputVars->push_back("s2_r37");
    inputVars->push_back("s2_r33");
    inputVars->push_back("s2_weta");
    inputVars->push_back("s1_emax2");
    inputVars->push_back("s1_delta");
    inputVars->push_back("s1_wtot");
    inputVars->push_back("s1_fracm");
    inputVars->push_back("s1_w1");
    inputVars->push_back("iso_trck");
    inputVars->push_back("iso_calo");
    // inputVars->push_back("status");
    
    factory->SetInputVariables( inputVars );    

    // sanity cuts
    TCut mycut;
    mycut += "abs(eta)>1.89 || abs(eta)<1.86";
    mycut += "abs(s1_wtot)<30";
    mycut += "abs(s1_fracm)<5";
    mycut += "abs(s1_w1)<5";
    mycut += "s2_r33>0";
    mycut += "s2_r37>0";      
    factory->PrepareTrainingAndTestTree( mycut, 4500, 5000 );
  }
  else if ((*((std::string*)config->value("DataSample")) == "Toy") || (*((std::string*)config->value("DataSample")) == "ToyMultiCut")) {

    cout << "==> perform 'Toy' analysis" << endl;

    // ascii files
    TString path = "data/";
    // TString datFileS = path + "sig_equal.dat";
    // TString datFileB = path + "bkg_equal.dat";
    //    TString datFileS = path + "sig_corr.dat";
    //    TString datFileB = path + "bkg_corr.dat";
//    TString datFileS = path + "sig_circ.dat";
//    TString datFileB = path + "bkg_circ.dat";
    TString datFileS = path + "toy_sig.dat";
    TString datFileB = path + "toy_bkg.dat";

    // create trees from these files
    if (!factory->SetInputTrees( datFileS, datFileB )) exit(1);

    inputVars->push_back("var1");
    inputVars->push_back("var2");
    inputVars->push_back("var3");
    inputVars->push_back("var4");

    factory->SetInputVariables( inputVars );    

    
    if (*((std::string*)config->value("DataSample")) == "ToyMultiCut") {
      // book several methods for different bins for var1
      Int_t nbins=2;
      Double_t array[5]={-5.,0.,5.};
      Int_t nbins2=2;
      Double_t array2[3]={-2.,0.,1.};

      factory->BookMultipleMVAs("var2", nbins, &array[0]);
      factory->BookMultipleMVAs("var4", nbins2, &array2[0]);
    }

    // sanity cuts
    TCut mycut = "";
//    factory->PrepareTrainingAndTestTree( mycut, *((int*)config->value("NTrainingEvents")), 10000 );    
    factory->PrepareTrainingAndTestTree( mycut, 10000, 10000 );    
  }
  else if (*((std::string*)config->value("DataSample")) == "ColorSuppressed") {

    cout << "==> perform 'ColorSuppressed' analysis" << endl;

    // ascii files
    TString path = "data/";
    TString datFileS = path + "SIG.ascii";
    TString datFileB = path + "OFF.ascii";

    // create trees from these files
    if (!factory->SetInputTrees( datFileS, datFileB )) exit(1);

    inputVars->push_back("L0");
    inputVars->push_back("L2");
    inputVars->push_back("R2");
    inputVars->push_back("Thrust");
    inputVars->push_back("Spher");
    inputVars->push_back("cosBmom");
    inputVars->push_back("cosThT");
    inputVars->push_back("costhr");
    inputVars->push_back("cosBSphr");
    
    factory->SetInputVariables( inputVars );    

    // sanity cuts
    TCut mycut = "";
    factory->PrepareTrainingAndTestTree( mycut, 5000, 10000 );    
  }
  else if (*((std::string*)config->value("DataSample")) == "Higgs") {

    cout << "==> perform 'Higgs' analysis" << endl;

    TFile *input      = new TFile("$ROOTSYS/tutorials/mlpHiggs.root");
    TTree *signal     = (TTree*)input->Get("sig_filtered");
    TTree *background = (TTree*)input->Get("bg_filtered");
    if (!factory->SetInputTrees( signal, background )) exit(1);
  
    //
    // Definition of input variables 
    inputVars->push_back("msumf");
    inputVars->push_back("ptsumf");
    inputVars->push_back("acolin");
    factory->SetInputVariables( inputVars );    
        
    //
    // Definition of cuts applied to all Trees
    factory->PrepareTrainingAndTestTree( "msumf > 0", -1 );    
  }
  else if (*((std::string*)config->value("DataSample")) == "HadronicCalib") {
    
    cout << "==> perform 'Local hadronic calibration' analysis" << endl;

    TFile *fsig       = new TFile("data/signal_new.root");
    TFile *fbg        = new TFile("data/background_new.root");

    TTree *signal     = (TTree*)fsig->Get("sigTree");
    TTree *background = (TTree*)fbg->Get("bgdTree");
    
    if (!factory->SetInputTrees( signal, background )) exit(1);

    //    inputVars->push_back("cl_e_topo");
    //  inputVars->push_back("cl_eemb0_topo");
    //  inputVars->push_back("cl_eemb1_topo");
    //  inputVars->push_back("cl_eemb2_topo");
    //  inputVars->push_back("cl_eemb3_topo");
    //  inputVars->push_back("cl_eta0_topo");
    //  inputVars->push_back("cl_eta1_topo");
    //  inputVars->push_back("cl_eta2_topo");
    //  inputVars->push_back("cl_eta3_topo");
    //  inputVars->push_back("cl_eeme0_topo");
    //  inputVars->push_back("cl_eeme1_topo");
    //  inputVars->push_back("cl_eeme2_topo");
    //  inputVars->push_back("cl_eeme3_topo");
    //  inputVars->push_back("cl_etileg1_topo");
    //  inputVars->push_back("cl_etileg2_topo");
    //  inputVars->push_back("cl_etileg3_topo");
    // inputVars->push_back("cl_eta_topo");
    // inputVars->push_back("cl_phi_topo");
    // begin moments
    //sven!

    inputVars->push_back("cl_m1_eta_topo");
    inputVars->push_back("cl_m1_phi_topo");
    inputVars->push_back("cl_m2_r_topo");
    inputVars->push_back("cl_m2_lambda_topo");
    inputVars->push_back("cl_delta_phi_topo");
    inputVars->push_back("cl_delta_theta_topo");
    inputVars->push_back("cl_delta_alpha_topo");
    inputVars->push_back("cl_center_x_topo");
    inputVars->push_back("cl_center_y_topo");
    inputVars->push_back("cl_center_z_topo");
    // sven
    inputVars->push_back("cl_center_lambda_topo");
    inputVars->push_back("cl_lateral_topo");
    inputVars->push_back("cl_longitudinal_topo");
    inputVars->push_back("cl_eng_frac_em_topo");
    inputVars->push_back("cl_eng_frac_max_topo");
    inputVars->push_back("cl_eng_frac_core_topo");
    inputVars->push_back("cl_m1_dens_topo");
    inputVars->push_back("cl_m2_dens_topo");
    //   inputVars->push_back("cl_ehec0_topo");
    //   inputVars->push_back("cl_ehec1_topo");
    //   inputVars->push_back("cl_ehec2_topo");
    //   inputVars->push_back("cl_ehec3_topo");
    //   inputVars->push_back("cl_etileb0_topo");
    //   inputVars->push_back("cl_etileb1_topo");
    //   inputVars->push_back("cl_etileb2_topo");
    //   inputVars->push_back("cl_etilee0_topo");
    //   inputVars->push_back("cl_etilee1_topo");
    //   inputVars->push_back("cl_etilee2_topo");
    //   inputVars->push_back("cl_efcal0_topo");
    //   inputVars->push_back("cl_efcal1_topo");
    // inputVars->push_back("cl_efcal2_topo");
    // inputVars->push_back("cl_ntotcells_topo");

    factory->SetInputVariables( inputVars );    
        
    factory->PrepareTrainingAndTestTree( "", 1000, 1000 );
  }

  else if (*((std::string*)config->value("DataSample")) == "DeleteBug" ) {

    cout << "==> perform 'Delete Bug ' analysis" << endl;

    TFile *input      = new TFile("$ROOTSYS/tutorials/mlpHiggs.root");
    TTree *signal     = (TTree*)input->Get("sig_filtered");
    TTree *background = (TTree*)input->Get("bg_filtered");

    TH1F *h  = new TH1F("h","test histogram",600,0,0.4);
    background->Draw("ptsumf>>h");
    cout<<"histogram entries = "<<h->GetEntries()<<endl;

    TMVA_PDF *pdf1 = new TMVA_PDF( h , TMVA_PDF::kSpline1 );
    TMVA_PDF *pdf2 = new TMVA_PDF( h , TMVA_PDF::kSpline2 );
    TMVA_PDF *pdf3 = new TMVA_PDF( h , TMVA_PDF::kSpline3 );
    TMVA_PDF *pdf4 = new TMVA_PDF( h , TMVA_PDF::kSpline5 );

    delete h;
    delete pdf1;
    delete pdf2;
    delete pdf3;
    delete pdf4;

    // exit normally
    cout<<"exit normally (hopefully)"<<endl;
    exit(0);
  }
  // --------- now do MV analysis ----------

  // ---- book MVA methods
  //
  // MethodCut options
  // format of option string: Method:nbin_effBvsSHist:nRandCuts:Option_var1:...:Option_varn
  // "Method" can be:
  //     - "MC"    : Monte Carlo optimization (recommended)
  //     - "FitSel": Minuit Fit: "Fit_Migrad" or "Fit_Simplex"
  //     - "FitPDF": PDF-based: only useful for uncorrelated input variables
  // "option_vari" can be 
  //     - "FMax"  : ForceMax   (the max cut is fixed to maximum of variable i)
  //     - "FMin"  : ForceMin   (the min cut is fixed to minimum of variable i)
  //     - "FSmart": ForceSmart (the min or max cut is fixed to min/max, based on mean value)
  //     - Adding "All" to "option_vari", eg, "AllFSmart" will use this option for all variables
  //     - if "option_vari" is empty (== ""), no assumptions on cut min/max are made

  if ( *((bool*)config->value("useCuts")) ) 
    factory->BookMethod( TMVA_Types::Cuts,  "MC:50000:AllFSmart" );
  
  // MethodLikelihood options:
  // histogram_interpolation_method:nsmooth:nsmooth:n_aveEvents_per_bin:Decorrelation
  if ( *((bool*)config->value("useLikelihood")) ) 
    factory->BookMethod( TMVA_Types::Likelihood, "Spline2:3"           ); 

  if ( *((bool*)config->value("useLikelihoodD")) ) 
    factory->BookMethod( TMVA_Types::Likelihood, "Spline2:10:25:D" ); 
 
  // MethodFisher:
  if ( *((bool*)config->value("useFisher")) ) 
    factory->BookMethod( TMVA_Types::Fisher,     "Fisher" ); // Fisher method ("Fi" or "Ma")
  
  // Method CF(Clermont-Ferrand)ANN:
  if ( *((bool*)config->value("useCFMlpANN")) ) 
    factory->BookMethod( TMVA_Types::CFMlpANN, "1000:N-1:N-2"  ); // n_cycles:#nodes:#nodes:...
  
  
  // Method CF(Root)ANN:
  if ( *((bool*)config->value("useTMlpANN")) ) 
    factory->BookMethod( TMVA_Types::TMlpANN,    "10:N-1:N-2"  ); // n_cycles:#nodes:#nodes:...
  
  // MethodHMatrix:
  if ( *((bool*)config->value("useHMatrix")) ) 
    factory->BookMethod( TMVA_Types::HMatrix ); // H-Matrix (chi2-squared) method
  
  // PDE - RS method
  // format and syntax of option string: "VolumeRangeMode:options"
  // where:
  //  VolumeRangeMode - all methods defined in private enum "VolumeRangeMode" 
  //  options         - deltaFrac in case of VolumeRangeMode=MinMax/RMS
  //                  - nEventsMin/Max, maxVIterations, scale for VolumeRangeMode=Adaptive
  if ( *((bool*)config->value("usePDERS")) ) 
    factory->BookMethod( TMVA_Types::PDERS, "Adaptive:100:200:50:0.99" ); 
    
  // MethodBDT (Boosted Decision Trees) options:
  // format and syntax of option string: "nTrees:BoostType:SeparationType:
  //                                      nEventsMin:minNodePurity:minSeparationgain:
  //                                      nCuts:SignalFraction"
  // nTrees:          number of trees in the forest to be created
  // BoostType:       the boosting type for the trees in the forest (AdaBoost e.t.c..)
  // SeparationType   the separation criterion applied in the node splitting
  // nEventsMin:      the minimum number of events in a node (leaf criteria, stop splitting)
  // SeparationGain:  the minimum gain in separation required in order to
  //                  continue splitting. !!
  //                  !!! Needs to be set to zero, as it doesn't work properly otherwise
  //                     ... it's strange though and not yet understood !!!
  // nCuts:  the number of steps in the optimisation of the cut for a node
  // SignalFraction:  scale parameter of the number of Bkg events  
  //                  applied to the training sample to simulate different initial purity
  //                  of your data sample. 
  //
  // known SeparationTypes are:
  //    - MisClassificationError
  //    - GiniIndex
  //    - CrossEntropy
  // known BoostTypes are:
  //    - AdaBoost
  //    - Bagging

  if ( *((bool*)config->value("useBDT")) ){
    char buffer[200];
    sprintf(buffer,"%d:%s:%s:%d:%f:%f:%d:%s",
	    *((int*)config->value("BDT::NTrees")),
	    ((std::string*)config->value("BDT::Boost"))->c_str(),
	    ((std::string*)config->value("BDT::Separation"))->c_str(),
	    *((int*)config->value("BDT::NodeMinEvents")),
	    *((double*)config->value("BDT::NodeMinSepGain")),
	    *((int*)config->value("BDT::NCuts"))
	    );
    factory->BookMethod( TMVA_Types::BDT, buffer );
  }
  // ---- train, test and evaluate the MVAs 
  // train MVAs
  factory->TrainAllMethods();
  // test MVAs
  factory->TestAllMethods();
  // evaluate MVAs
  //  factory->EvaluateAllVariables();
  // evaluate variables
  factory->EvaluateAllMethods();    
  if (*((std::string*)config->value("DataSample")) == "ToyMultiCut") {
    // process Multi MVAs
    factory->ProcessMultipleMVA();
    
  }
  
  target->Close();

  // clean up
  delete factory;
  delete inputVars;
  
  cout << "==> wrote root file TMVAnalysis.root" << endl;
  cout << "==> TMVAnalysis is done!" << endl;      
} 
