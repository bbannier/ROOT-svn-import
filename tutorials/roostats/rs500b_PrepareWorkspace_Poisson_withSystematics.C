//////////////////////////////////////////////////////////////////////////
//
// RooStats tutorial macro #500b
// 2009/08 - Nils Ruthmann, Gregory Schott
//
// Prepare a workspace (stored in a ROOT file) containing a models,
// data and other objects needed to run statistical classes in
// RooStats.
//
// In this macro a PDF model is built for a counting analysis.  A
// certain number of events are observed (this can be enforced or left
// free) while a number of background events is expected.  It is also
// assumed there is a systematic uncertainty on the number of expected
// background events.  The parameter of interest is the signal yield
// and we assume for it a flat prior.  All needed objects are stored
// in a ROOT file (within a RooWorkspace container); this ROOT file
// can then be fed as input to various statistical methods.
//
// root -q -x -l 'rs500b_PrepareWorkspace_Poisson_withSystematics.C()'
//
//////////////////////////////////////////////////////////////////////////

using namespace RooFit;
using namespace RooStats;


void rs500b_PrepareWorkspace_Poisson_withSystematics( TString fileName = "WS_Poisson_withSystematics.root" )
{

  // use a RooWorkspace to store the pdf models, prior informations, list of parameters,...
  RooWorkspace myWS("myWS");

  // Observable
  myWS.factory("x[0,0,1]") ;

  // Pdf in observable, 
  myWS.factory("Uniform::sigPdf(x)") ;
  myWS.factory("Uniform::bkgPdf(x)") ;
  myWS.factory("SUM::model(S[100,0,1500]*sigPdf,B[1000,0,3000]*bkgPdf") ;

  // Background only pdf
  myWS.factory("ExtendPdf::modelBkg(bkgPdf,B)") ;

  // Priors
  myWS.factory("Gaussian::priorNuisance(B,1000,200)") ;
  myWS.factory("Uniform::priorPOI(S)") ;

  // Definition of observables and parameters of interest
  myWS.defineSet("observables","x");
  myWS.defineSet("POI","S");
  myWS.defineSet("parameters","B") ;
  
  // Generate data
  RooAbsData* data = myWS.pdf("model")->generateBinned(*myWS.set("observables"),Name("data"),ExpectedData());
  myWS.import(*data) ;


  myWS.writeToFile(fileName);  
  std::cout << "\nRooFit model initialized and stored in " << fileName << std::endl;
}
