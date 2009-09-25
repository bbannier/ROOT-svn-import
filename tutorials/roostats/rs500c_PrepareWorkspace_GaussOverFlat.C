//////////////////////////////////////////////////////////////////////////
//
// RooStats tutorial macro #500c
// 2009/08 - Nils Ruthmann, Gregory Schott
//
// Prepare a workspace (stored in a ROOT file) containing a models,
// data and other objects needed to run statistical classes in
// RooStats.
//
// In this macro a PDF model is built assuming signal has a Gaussian
// PDF and the background a flat PDF.  The parameter of interest is
// the signal yield and we assume for it a flat prior.  In this macro,
// no systematic uncertainty is considered (see
// rs500d_PrepareWorkspace_GaussOverFlat_withSystematics.C).  All needed
// objects are stored in a ROOT file (within a RooWorkspace
// container); this ROOT file can then be fed as input to various
// statistical methods.
//
// root -q -x -l 'rs500c_PrepareWorkspace_GaussOverFlat.C()'
//
//////////////////////////////////////////////////////////////////////////

using namespace RooFit;
using namespace RooStats;


void rs500c_PrepareWorkspace_GaussOverFlat( TString fileName = "WS_GaussOverFlat.root" )
{
  // use a RooWorkspace to store the pdf models, prior informations, list of parameters,...
  RooWorkspace myWS("myWS");

  // Observable
  myWS.factory("mass[0,500]") ;

  // Pdf in observable, 
  myWS.factory("Gaussian::sigPdf(mass,200,50)") ;
  myWS.factory("Uniform::bkgPdf(mass)") ;
  myWS.factory("SUM::model(S[0,0,60]*sigPdf,B[10]*bkgPdf") ;

  // Background only pdf
  myWS.factory("ExtendPdf::modelBkg(bkgPdf,B)") ;

  // Priors
  myWS.factory("Uniform::priorPOI(S)") ;

  // Definition of observables and parameters of interest
  myWS.defineSet("observables","mass");
  myWS.defineSet("POI","S");
  
  // Generate data
  RooAbsData* data = myWS.pdf("model")->generateBinned(*myWS.set("observables"),Name("data"),ExpectedData());
  myWS.import(*data) ;

  myWS.writeToFile(fileName);  
  std::cout << "\nRooFit model initialized and stored in " << fileName << std::endl;

}
