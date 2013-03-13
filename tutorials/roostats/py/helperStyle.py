
import ROOT

ROOT.gStyle.SetOptStat( 0 )
ROOT.gStyle.SetHistLineWidth( 2 )
# Set the paper and margin sizes:
ROOT.gStyle.SetPaperSize( 20, 26 )
ROOT.gStyle.SetPadTopMargin( 0.07 )
ROOT.gStyle.SetPadRightMargin( 0.05 )
ROOT.gStyle.SetPadBottomMargin( 0.16 )
ROOT.gStyle.SetPadLeftMargin( 0.16 )
# set title offsets (for axis label)
ROOT.gStyle.SetTitleXOffset(1.4);
ROOT.gStyle.SetTitleYOffset(1.4);
# Put tick marks on top and rhs of the plots:
ROOT.gStyle.SetPadTickX( 1 )
ROOT.gStyle.SetPadTickY( 1 )
