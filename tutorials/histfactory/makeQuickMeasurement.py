#!/usr/bin/env python

#
# A pyROOT script that allows one to
# make quick measuremenst.
#
# This is a command-line script that
# takes in signal and background values,
# as well as potentially uncertainties on those
# values, and returns a fitted signal value
# and errors

def main():
    """ Create a simple model and run statistical tests  

    This script can be used to make simple statistical measurements.
    It takes values for signal, background, and data as input, and
    can optionally take uncertainties on signal or background.
    It returns a fitted value for the signal strength, which is
    defined as:  FittedSignal = NominalSignal * SignalStrength 

    """
    
    # Let's parse the input
    # Define the command line options of the script:
    import optparse
    desc = " ".join(main.__doc__.split())    

    vers = "0.1"
    parser = optparse.OptionParser( description = desc, version = vers, usage = "%prog [options]" )

    parser.add_option( "-s", "--signal", dest = "signal",
                       action = "store", type = "float", default=None,
                       help = "Expected Signal" )

    parser.add_option( "-b", "--background", dest = "background",
                       action = "store", type = "float", default=None,
                       help = "Expected Background" )

    parser.add_option( "-d", "--data", dest = "data",
                       action = "store", type = "float", default=None,
                       help = "Measured data" )

    parser.add_option( "--signal-uncertainty", dest = "signal_uncertainty",
                       action = "store", type = "float", default=None,
                       help = "Uncertainty on the signal rate in percentage. --signal-uncertainty=5 means a 5% uncertainty." )
 
    parser.add_option( "--background-uncertainty", dest = "background_uncertainty",
                       action = "store", type = "float", default=None,
                       help = "Uncertainty on the background rate in percentage. --background-uncertainty=5 means a 5% uncertainty." )

    parser.add_option( "--output-prefix", dest = "output_prefix",
                       action = "store", type = "string", default="Measurement",
                       help = "Prefix for output files when using export.  Can include directories (ie 'MyDirectory/MyPrefix')" )

    parser.add_option( "-e", "--export", dest = "export",
                       action = "store_true", default=False,
                       help = "Make output plots, graphs, and save the workspace." )

    parser.add_option( "-t", "--test", dest = "test",
                       action = "store", default=None,
                       help = """Run a standard statistical test on the newly created model.
                               Options are: 'ProfileLikelihood',  Make output plots, graphs, and save the workspace.""" )

    # Parse the command line options:
    ( options, unknown ) = parser.parse_args()
    
    # Make a log
    # Set the format of the log messages:
    FORMAT = 'Py:%(name)-25s  %(levelname)-8s  %(message)s'
    import logging
    logging.basicConfig( format = FORMAT )
    # Create the logger object:
    logger = logging.getLogger( "makeQuickMeasurement" )
    # Set the following to DEBUG when debugging the scripts:
    logger.setLevel( logging.INFO )
    
    # So a small sanity check:
    if len( unknown ):
        logger.warning( "Options(s) not recognised: [" + ",".join( unknown ) + "]" )

    # Ensure that all necessary input has been supplied
    if options.signal == None:
        logger.error( "You have to define a value for expacted signal (use --signal)" )
        return 255

    if options.background == None:
        logger.error( "You have to define a value for expacted background (use --background)" )
        return 255

    if options.data == None:
        logger.error( "You have to define a value for measured data (use --data)" )
        return 255


    # If we are running a test, check that we have acceptable input
    test_options = [ 'ProfileLikelihood', 'BayesianMCMC', 'OneSidedFrequentistUpperLimitWithBands', 'FeldmanCousins' ]
    if options.test != None and options.test not in test_options:
        logger.error("Please supply an acceptable test as an option.  Possible choices are:")
        for test in test_options:
            logger.error( test )
        return

    
    # Okay, if all input is acceptable, we simply pass
    # it to the MakeSimpleMeasurement function, which
    # does the real work.

    MakeSimpleMeasurement( signal_val=options.signal, background_val=options.background, data_val=options.data,
                           signal_uncertainty=options.signal_uncertainty, background_uncertainty=options.background_uncertainty,
                           Export=options.export, output_prefix=options.output_prefix, test=options.test )

    return


def MakeSimpleMeasurement( signal_val, background_val, data_val, signal_uncertainty=None, background_uncertainty=None, 
                           Export=False, output_prefix="Measurement", test=None ):
    """ Make a simple measurement using HistFactory
    
    Take in simple values for signal, background data, 
    and potentially uncertainty on signal and background

    """

    try:
        import ROOT
    except:
        print "It seems that pyROOT isn't properly configured"
        return

    # Create and name a measurement
    # Set the Parameter of interest, and set several
    # other parameters to be constant
    meas = ROOT.RooStats.HistFactory.Measurement("meas", "meas")
    meas.SetOutputFilePrefix( output_prefix )
    meas.SetPOI( "SigXsecOverSM" )

    # We don't include Lumi here, 
    # but since HistFactory gives it to 
    # us for free, we set it constant
    # The values are just dummies

    meas.SetLumi( 1.0 )
    meas.SetLumiRelErr( 0.10 )
    meas.AddConstantParam("Lumi")

    # We set ExportOnly to false.  This parameter
    # defines what happens when we run MakeMeasurementAndModelFast
    # If we DO run that function, we also want it to export.
    meas.SetExportOnly( False )

    # Create a channel and set
    # the measured value of data 
    # (no extenal hist necessar for cut-and-count)
    chan = ROOT.RooStats.HistFactory.Channel( "channel" )
    chan.SetData( data_val )

    # Create the signal sample and set it's value
    signal = ROOT.RooStats.HistFactory.Sample( "signal" )
    signal.SetNormalizeByTheory( False )
    signal.SetValue( signal_val )
    #signal.SetValue( 10 )

    # Add the parmaeter of interest and a systematic
    signal.AddNormFactor( "SigXsecOverSM", 1, 0, 3 )

    # If we have a signal uncertainty, add it too
    if signal_uncertainty != None:
        uncertainty_up   = 1.0 + signal_uncertainty/100.0
        uncertainty_down = 1.0 - signal_uncertainty/100.0
        signal.AddOverallSys( "signal_uncertainty",  uncertainty_down, uncertainty_up )

    # Finally, add this sample to the channel
    chan.AddSample( signal )

    # Create a background sample
    background = ROOT.RooStats.HistFactory.Sample( "background" )
    background.SetNormalizeByTheory( False )
    background.SetValue( background_val )

    # If we have a background uncertainty, add it too
    if background_uncertainty != None:
        uncertainty_up   = 1.0 + background_uncertainty/100.0
        uncertainty_down = 1.0 - background_uncertainty/100.0
        background.AddOverallSys( "background_uncertainty",  uncertainty_down, uncertainty_up )

    # Finally, add this sample to the channel
    chan.AddSample( background )

    # Add this channel to the measurement
    # There is only this one channel, after all
    meas.AddChannel( chan )

    # Now, do the measurement
    if Export: 
        workspace = ROOT.RooStats.HistFactory.MakeModelAndMeasurementFast( meas )
        return workspace

    else:
        factory = ROOT.RooStats.HistFactory.HistoToWorkspaceFactoryFast()
        workspace = factory.MakeCombinedModel( meas )
        #workspace = ROOT.RooStats.HistFactory.HistoToWorkspaceFactoryFast.MakeCombinedModel( meas )
        ROOT.RooStats.HistFactory.FitModel( workspace )

    # Run a statistical test if requred
    if test != None:

        # First, for this to work from PyROOT,
        # we have to load some libraries "by hand"
        ROOT.gSystem.Load("libCint.so")
        ROOT.gSystem.Load("libRint.so")

        if test == "":
            print "Error: You must supply a test"

        elif test == "ProfileLikelihood": 
            ROOT.gROOT.LoadMacro("$ROOTSYS/tutorials/roostats/StandardProfileLikelihoodDemo.C+")
            ROOT.RooStats.StandardProfileLikelihoodDemo( workspace )

        elif test == "BayesianMCMC": 
            ROOT.gROOT.LoadMacro("$ROOTSYS/tutorials/roostats/StandardBayesianMCMCDemo.C+")
            ROOT.RooStats.StandardBayesianMCMCDemo( workspace )

        elif test == "OneSidedFrequentistUpperLimitWithBands": 
            ROOT.gROOT.LoadMacro("$ROOTSYS/tutorials/roostats/OneSidedFrequentistUpperLimitWithBands.C+")
            ROOT.RooStats.OneSidedFrequentistUpperLimitWithBands( workspace )

        elif test == "FeldmanCousins": 
            ROOT.gROOT.LoadMacro("$ROOTSYS/tutorials/roostats/StandardFeldmanCousinsDemo.C+")
            ROOT.RooStats.StandardFeldmanCousinsDemo( workspace )

        else:
            print "Error: You must supply a valid test name"

    # At this point, we are done
    return


if __name__ == "__main__":
    main()
