#!/usr/bin/env python

#  Created on: February 5, 2013

__author__ = "Sven Kreiss, Kyle Cranmer"
__version__ = "0.1"


import optparse
parser = optparse.OptionParser(version="0.1")
parser.add_option("-i", "--input", help="root file", type="string", dest="input", default="results/example_combined_GaussExample_model.root")
parser.add_option("-o", "--output", help="output location", type="string", dest="output", default="docImages/")

parser.add_option("-w", "--wsName", help="Workspace name", type="string", dest="wsName", default="combined")
parser.add_option("-m", "--mcName", help="ModelConfig name", type="string", dest="mcName", default="ModelConfig")
parser.add_option("-d", "--dataName", help="data name", type="string", dest="dataName", default="obsData")

parser.add_option("-f", "--fullRun", help="Do a full run.", dest="fullRun", default=False, action="store_true")

parser.add_option("-q", "--quiet", dest="verbose", action="store_false", default=True, help="Quiet output.")
options,args = parser.parse_args()


import ROOT
ROOT.gROOT.SetBatch( True )
import helperStyle

import PyROOTUtils
import math
from array import array



def getContours( hist, level, canvas ):
   hist.SetContour( 1, array('d',[level]) )
   hist.Draw( "CONT LIST" )
   canvas.Update()
   listOfGraphs = ROOT.gROOT.GetListOfSpecials().FindObject("contours").At(0)
   contours = [ ROOT.TGraph( listOfGraphs.At(i) ) for i in range( listOfGraphs.GetSize() ) ]
   for co in range( len(contours) ):
      contours[co].SetLineWidth( 2 )
      contours[co].SetLineColor( ROOT.kBlue )
      contours[co].SetName( "Contour%.0fTG_%d" % (level*100,co) )
   return contours



def main():
   
   ROOT.RooRandom.randomGenerator().SetSeed( 0 )
   
   file = ROOT.TFile.Open(options.input)
   if not file:
      # Normally this would be run on the command line
      print( "will run standard hist2workspace example" )
      ROOT.gROOT.ProcessLine(".! prepareHistFactory .")
      ROOT.gROOT.ProcessLine(".! hist2workspace config/example.xml")
      print( "\n\n---------------------" )
      print( "Done creating example input" )
      print( "---------------------\n\n" )
      file = ROOT.TFile.Open("results/example_combined_GaussExample_model.root")
      
   if not file:
      # if it is still not there, then we can't continue
      print( "Not able to run hist2workspace to create example input" )

  
   # /////////////////////////////////////////////////////////////
   # // Tutorial starts here
   # ////////////////////////////////////////////////////////////
   
   w = file.Get( options.wsName )
   if not w:
      print( "ERROR: Workspace "+options.wsName+" not found." )
      return
   mc = w.obj( options.mcName )
   if not mc:
      print( "ERROR: ModelConfig "+options.mcName+" not found." )
      return
   data = w.data( options.dataName )
   if not data:
      print( "ERROR: Data "+options.dataName+" not found." )
      return


   firstPOI = mc.GetParametersOfInterest().first()
   firstPOI.setMax(10.)
   listNuisPars = ROOT.RooArgList( mc.GetNuisanceParameters() )


   # We want to create an overview using the following proposal functions
   proposalFunctions = [
         { "proposal": ROOT.RooStats.SequentialProposal(10), 
           "id"      : "SequentialProposal",
           "title"   : "Standard configuration" },
   ]
   if options.fullRun:
      proposalFunctions += [
         { "proposal": ROOT.RooStats.SequentialProposal(100), 
           "id"      : "SequentialProposal_100",
           "title"   : "Divisor = 100" },
         { "proposal": ROOT.RooStats.SequentialProposal(10, mc.GetParametersOfInterest(), 3), 
           "id"      : "SequentialProposal_10_03",
           "title"   : "Divisor = 10, Importance factor of POI = 3" },
         { "proposal": ROOT.RooStats.SequentialProposal(10, mc.GetParametersOfInterest(), 10), 
           "id"      : "SequentialProposal_10_10",
           "title"   : "Divisor = 10, Importance factor of POI = 10" },
      ]
   
   
   mcmc = ROOT.RooStats.MCMCCalculator(data,mc)
   mcmc.SetConfidenceLevel(0.95) # 95% interval
   mcmc.SetNumIters(100000)          # Metropolis-Hastings algorithm iterations
   mcmc.SetLeftSideTailFraction(0);  # for one-sided Bayesian interval

   for pF in proposalFunctions:
      print( "\n\n---------- "+pF["title"]+" ----------------" )
      mcmc.SetProposalFunction( pF["proposal"] )
      interval = mcmc.GetInterval()
      interval.SetNumBurnInForFractionOfEntries( 0.3 )
      #interval.SetNumBurnInForNumEntries( 10000 )

      # print out the iterval on the first Parameter of Interest
      print( "\n95%% interval on %s is [%f,%f]" % (
         firstPOI.GetName(),
         interval.LowerLimit(firstPOI),
         interval.UpperLimit(firstPOI)
      ) )

      c1 = ROOT.TCanvas(pF["id"]+"_intervalPlot", pF["id"]+"_intervalPlot", 500, 250)
      plot = ROOT.RooStats.MCMCIntervalPlot(interval)
      plot.Draw()
      c1.SaveAs( options.output+pF["id"]+"_interval.png" )

      c2 = ROOT.TCanvas(pF["id"]+"_POIAndFirstNuisParWalk", pF["id"]+"_POIAndFirstNuisParWalk", 500, 250)
      plot.DrawChainScatter( firstPOI, listNuisPars.at(0) )
      c2.SaveAs( options.output+pF["id"]+"_POIAndFirstNuisParWalk.png" )
      
      c3 = ROOT.TCanvas(pF["id"]+"_extraPlots", pF["id"]+"_extraPlots", 4800, 3200)
      c3.Divide( 5, listNuisPars.getSize() )
      cont = []
      # draw a scatter plot of chain results for poi vs each nuisance parameters
      for i in range( listNuisPars.getSize() ):
         c3.cd( i*5 + 1 )
         plot.DrawChainScatter( firstPOI, listNuisPars.at(i) )

         c3.cd( i*5 + 2 )
         h4 = plot.GetMinNLLHist1D( listNuisPars.at(i) )
         if h4.GetMaximum() > 15: h4.SetMaximum( 15 )
         h4.Draw( "HIST" )
         cont.append( h4 )

         c3.cd( i*5 + 3 )
         h2 = plot.GetHist1D( listNuisPars.at(i) )
         h2.SetTitle( "Comparison of Posterior (red) and Likelihood Shape (blue)" )
         h2.GetYaxis().SetTitle( "Posterior (red) / Likelihood Shape (blue)" )
         h2.SetLineColor( ROOT.kRed )
         h2.Scale( 1./(h2.Integral()) )
         h2.Draw( "HIST" )
         cont.append( h2 )
         h3 = plot.GetMaxLikelihoodHist1D( listNuisPars.at(i) )
         h3.SetLineColor( ROOT.kBlue )
         h3.Scale( 1./h3.Integral() )
         h3.Draw( "HIST SAME" )
         cont.append( h3 )
         c3.Update()

         c3.cd( i*5 + 4 )
         h1 = plot.GetMinNLLHist2D( firstPOI, listNuisPars.at(i) )
         cont.append( h1 )
         cont68Profile = getContours( h1, 2.3/2.0, c3 )
         cont95Profile = getContours( h1, 6.0/2.0, c3 )
         cont.append( cont68Profile+cont95Profile )

         h5 = plot.GetHist2D( firstPOI, listNuisPars.at(i) )
         h5.SetTitle( "Comparison of Posterior Contours (red) and NLL Contours (blue)" )
         h5.GetZaxis().SetTitle( "Posterior Contours (red) / NLL Contours (blue)" )
         cont.append( h5 )
         cont68Marginalized = getContours( h5, ROOT.RooStats.ContourLevelHPD(h5,0.68), c3 )
         cont95Marginalized = getContours( h5, ROOT.RooStats.ContourLevelHPD(h5,0.95), c3 )
         cont.append( cont68Marginalized+cont95Marginalized )

         for c in cont68Profile:
            c.SetLineColor( ROOT.kBlue )            
            c.Draw( "SAME" )
         for c in cont95Profile:
            c.SetLineStyle( ROOT.kDashed )
            c.SetLineColor( ROOT.kBlue )            
            c.Draw( "SAME" )
         for c in cont68Marginalized:
            c.SetLineColor( ROOT.kRed )            
            c.Draw( "SAME" )
         for c in cont95Marginalized:
            c.SetLineStyle( ROOT.kDashed )
            c.SetLineColor( ROOT.kRed )            
            c.Draw( "SAME" )

         c3.cd( i*5 + 5 )
         h6 = plot.GetMaxLikelihoodHist2D( firstPOI, listNuisPars.at(i) )
         h6.Scale( 1./h6.Integral() )
         h6.Draw( "COLZ" )
         cont.append( h6 )
         c3.Update()

      c3.SaveAs( options.output+pF["id"]+"_extras.png" )

      c4 = ROOT.TCanvas(pF["id"]+"_NLLTimeDev", pF["id"]+"_NLLTimeDev", 400, 400)
      tgNLLVsTime = plot.GetNLLVsTime( 1000 )
      tgNLLVsTime.Draw("AL")
      c4.SaveAs( options.output+pF["id"]+"_NLLTimeDev_1000Samples.png" )
      tgNLLVsTime = plot.GetNLLVsTime()
      tgNLLVsTime.Draw("AL")
      c4.SaveAs( options.output+pF["id"]+"_NLLTimeDev_allSamples.png" )

      c5 = ROOT.TCanvas(pF["id"]+"_POIVsTime", pF["id"]+"_POIVsTime", 400, 400)
      tgPOIVsTime = plot.GetParameterVsTime( firstPOI, 1000 )
      tgPOIVsTime.Draw("AL")
      c5.SaveAs( options.output+pF["id"]+"_POIVsTime_1000Samples.png" )
      tgPOIVsTime = plot.GetParameterVsTime( firstPOI )
      tgPOIVsTime.Draw("AL")
      c5.SaveAs( options.output+pF["id"]+"_POIVsTime_allSamples.png" )
      

if __name__ == "__main__":
   main()
