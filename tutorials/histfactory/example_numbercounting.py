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
# In this example, the model is configured and built
# entirely within this script.  It requires no external
# configuration files or any saved histograms.
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

    # Create and name a measurement
    meas = ROOT.RooStats.HistFactory.Measurement("meas", "meas")

    #meas.SetOutputFilePrefix( "./results/example_UsingPy_SetVal" )

    # Set the Parameter of interest, and set several
    # other parameters to be constant
    meas.SetPOI( "SigXsecOverSM" )
    meas.AddConstantParam("Lumi")
    meas.AddConstantParam("alpha_syst1")
    meas.AddConstantParam("alpha_syst2")
    meas.AddConstantParam("alpha_syst3")

    # Set the Lumi (1.0 = nominal in this example, 10% error)
    meas.SetLumi( 1.0 )
    meas.SetLumiRelErr( 0.10 )

    # Here, this determines if the model is fit
    # within the "MakeModelFast" function or not
    meas.SetExportOnly( False )


    # InputFile = "./data/NumberCounting.root"

    # Create a channel and set
    # the measured value of data 
    # (no extenal hist necessar for cut-and-count)
    chan = ROOT.RooStats.HistFactory.Channel( "channel1" )
    chan.SetData( 20 )
    chan.SetStatErrorConfig( 0.05, "Poisson" )

    # Create the signal sample and
    # set it's value
    signal = ROOT.RooStats.HistFactory.Sample( "signal" )
    signal.SetValue( 5 )
    # Add the parmaeter of interest and a systematic
    signal.AddNormFactor( "SigXsecOverSM", 1, 0, 3 )
    signal.AddOverallSys( "syst1",  0.95, 1.05 )
    chan.AddSample( signal )


    # Create a background sample
    background1 = ROOT.RooStats.HistFactory.Sample( "background1" )
    background1.SetValue( 10 )
    background1.SetNormalizeByTheory( True )
    # Add a systematic
    background1.AddOverallSys( "syst2", 0.95, 1.05  )
    chan.AddSample( background1 )


    # Create another background sample
    background2 = ROOT.RooStats.HistFactory.Sample( "background2" )
    background2.SetValue( 4 )
    background2.SetNormalizeByTheory( True )
    # Add a systematic
    background2.AddOverallSys( "syst3", 0.95, 1.05  )
    chan.AddSample( background2 )


    # Add this channel to the measurement
    meas.AddChannel( chan )

    # Print some info for debugging
    meas.PrintTree()

    # Now, do the measurement
    myFactory = ROOT.RootStats.HistFactory()
    myWorkspace = myFactory.MakeCombinedModel( measurement )

    combinedWorkspace = ROOT.RooStats.HistFactory.MakeModelAndMeasurementFast( meas )

    #combinedWorkspace = ROOT.RooStats.HistFactory.MakeModelFast( meas )
    #combinedWorkspace = ROOT.RooStats.HistFactory.MakeModelFast( meas )

    # combinedWorkspace.Print("V")

    pass


if __name__ == "__main__":
    main()
