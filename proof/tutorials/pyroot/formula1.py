#
# To see the graphics output of this macro, click begin_html <a href="gif/formula1.gif">here</a>. end_html
#

from ROOT import TCanvas, TFormula, TF1
from ROOT import gROOT, gObjectTable

gROOT.Reset()

c1 = TCanvas( 'c1', 'Example with Formula', 200, 10, 700, 500 )

# We create a formula object and compute the value of this formula
# for two different values of the x variable.
form1 = TFormula( 'form1', 'sqrt(abs(x))' )
form1.Eval( 2 )
form1.Eval( -45 )

# Create a one dimensional function and draw it
fun1 = TF1( 'fun1', 'abs(sin(x)/x)', 0, 10 )
c1.SetGridx()
c1.SetGridy()
fun1.Draw()
c1.Update()

# Before leaving this demo, we print the list of objects known to ROOT
#
if ( gObjectTable ):
    gObjectTable.Print()
