/*
 * StandardMultiNestDemo.C
 *
 *  Created on: Oct 10, 2011
 *      Author: Sven Kreiss, Kyle Cranmer
 */


#include "TFile.h"
#include "TStopwatch.h"
#include "RooStats/ModelConfig.h"
#include "RooStats/MultiNestCalculator.h"
#include "RooStats/MCMCInterval.h"
#include "RooWorkspace.h"

#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAbsReal.h"

#include "TROOT.h"
#include "TStyle.h"

using namespace RooFit;
using namespace RooStats;



void StandardMultiNestDemo( const char* infile = "",
                            const char* workspaceName = "combined",
                            const char* modelConfigName = "ModelConfig",
                            const char* dataName = "obsData",
                            double tol = 0.1,
                            double efr = 0.3 )
{

   /////////////////////////////////////////////////////////////
   // First part is just to access a user-defined file
   // or create the standard example file if it doesn't exist
   ////////////////////////////////////////////////////////////
   const char* filename = "";
   if (!strcmp(infile, "")) filename = "results/example_combined_GaussExample_model.root";
   else filename = infile;

   // Check if example input file exists
   TFile *file = TFile::Open(filename);

   // if input file was specified byt not found, quit
   if (!file && strcmp(infile, "")) {
      cout << "file not found" << endl;
      return;
   }

   // if default file not found, try to create it
   if (!file) {
      // Normally this would be run on the command line
      cout << "will run standard hist2workspace example" << endl;
      gROOT->ProcessLine(".! prepareHistFactory .");
      gROOT->ProcessLine(".! hist2workspace config/example.xml");
      cout << "\n\n---------------------" << endl;
      cout << "Done creating example input" << endl;
      cout << "---------------------\n\n" << endl;
   }

   // now try to access the file again
   file = TFile::Open(filename);
   if (!file) {
      // if it is still not there, then we can't continue
      cout << "Not able to run hist2workspace to create example input" << endl;
      return;
   }


   /////////////////////////////////////////////////////////////
   // Now get the data and workspace
   ////////////////////////////////////////////////////////////

   // get the workspace out of the file
   RooWorkspace* w = (RooWorkspace*) file->Get(workspaceName);
   if (!w) {
      cout << "workspace not found" << endl;
      return;
   }

   // get the modelConfig out of the file
   ModelConfig* mc = (ModelConfig*) w->obj(modelConfigName);

   // get the modelConfig out of the file
   RooAbsData* data = w->data(dataName);

   // make sure ingredients are found
   if (!data || !mc) {
      w->Print();
      cout << "data or ModelConfig was not found" << endl;
      return;
   }

   mc->Print();
   data->Print();



   /////////////////////////////////////////////////////////////
   // Now get the data and workspace
   ////////////////////////////////////////////////////////////

   RooRealVar *poi = (RooRealVar*)mc->GetParametersOfInterest()->first();
   poi->setRange(0,10);
   poi->Print();

   // configure calculator
   MultiNestCalculator m;
   m.SetModel( *mc );
   m.SetData( *data );
   m.SetConfidenceLevel( 0.95 );    // 95% credibility interval
   m.SetLeftSideTailFraction( 0 );  // upper limit
   //m.SetPosteriorParameters( *mc->GetParametersOfInterest() );
   m.Ceff( kFALSE );
   m.Efr( efr );
   m.MModal( kTRUE );
   m.MaxModes( 200 );
   m.Nlive( 1000 );
   m.Tol( tol );


   // get interval and store result
   MultiNestInterval* interval = m.GetInterval();
   if (interval) {
      MultiNestIntervalPlot* plot = new MultiNestIntervalPlot( *interval );
      //plot->GetHist1D( *poi )->Draw();
      plot->Draw();
      
      cout << endl;
      cout << "--- MultiNest interval ---" << endl;
      cout << "  interval: [ " << interval->LowerLimit( *poi ) << ", " << interval->UpperLimit( *poi ) << " ]" << endl;
      cout << "  actual confidence level: " << interval->GetActualConfidenceLevel() << endl;
      cout << endl;
   }
}


