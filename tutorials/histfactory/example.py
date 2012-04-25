#!/usr/bin/env python

#
# A pyROOT script demonstrating
# an example of writing a HistFactory
# model using python
#
# This example was written to match
# the example.xml analysis in
# $ROOTSYS/tutorials/histfactory/
#
# Written by George Lewis
#

#
# There are several way to use HistFactory to
# create a model.
#
# In this example, we configure the model using PyRoot
# and load the histograms from an external ROOT file
#


def main():

    try:
        import ROOT
    except:
        print "It seems that pyROOT isn't properly configured"
        return

    """
    Create a HistFactory measurement from python
    """

    InputFile = "./data/example.root"

    # Create the measurement
    meas = ROOT.RooStats.HistFactory.Measurement("meas", "meas")

    meas.SetOutputFilePrefix( "./results/example_UsingPy" )
    meas.SetPOI( "SigXsecOverSM" )
    meas.AddConstantParam("Lumi")
    meas.AddConstantParam("alpha_syst1")

    meas.SetLumi( 1.0 )
    meas.SetLumiRelErr( 0.10 )
    meas.SetExportOnly( False )

    # Create a channel

    chan = ROOT.RooStats.HistFactory.Channel( "channel1" )
    chan.SetData( "data", InputFile )
    chan.SetStatErrorConfig( 0.05, "Poisson" )

    # Now, create some samples

    # Create the signal sample
    signal = ROOT.RooStats.HistFactory.Sample( "signal", "signal", InputFile )
    signal.AddOverallSys( "syst1",  0.95, 1.05 )
    signal.AddNormFactor( "SigXsecOverSM", 1, 0, 3 )
    chan.AddSample( signal )


    # Background 1
    background1 = ROOT.RooStats.HistFactory.Sample( "background1", "background1", InputFile )
    background1.ActivateStatError( "background1_statUncert", InputFile )
    background1.AddOverallSys( "syst2", 0.95, 1.05  )
    chan.AddSample( background1 )


    # Background 1
    background2 = ROOT.RooStats.HistFactory.Sample( "background2", "background2", InputFile )
    background2.ActivateStatError()
    background2.AddOverallSys( "syst3", 0.95, 1.05  )
    chan.AddSample( background2 )


    # Done with this channel
    # Add it to the measurement:
    meas.AddChannel( chan )

    # At this point, all histograms information is stored as
    # the string name of the root file and histogram
    # We must now open those files and load the histograms:
    meas.CollectHistograms()

    # Print some output for debugging
    meas.PrintTree();

    # One can print XML code to an
    # output directory.
    # Here, the directory is "xmlFromPy"
    meas.PrintXML( "xmlFromPy", meas.GetOutputFilePrefix() );

    # Now, do the measurement and make output
    # combinedWorkspace = ROOT.RooStats.HistFactory.MakeModelAndMeasurementFast( meas );

    print "Making Measurement"
    factory = ROOT.RooStats.HistFactory.HistoToWorkspaceFactoryFast()
    combinedWorkspace = factory.MakeCombinedModel( meas )
    print "Successfully Made Measurement"

    print "Printing Workspace"
    combinedWorkspace.Print()
    print "Successfully printed workspace"


    # combinedWorkspace = ROOT.RooStats.HistFactory.HistToWorkspaceFactoryFastMakeModelFast( meas );

    # combinedWorkspace.Print("V")

    pass


if __name__ == "__main__":
    main()
