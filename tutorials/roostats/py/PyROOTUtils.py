#!/usr/bin/env python

__author__ = "Sven Kreiss, Kyle Cranmer"
__version__ = "0.1"
__doc__ = """
Module providing some convenience classes for ROOT. This helps making fixed font size
legends, graphs from Python lists, Bands from lists including outlines of Bands. One
of the highlights is also putting text on graphs including multi-line support; and
it is just a single function call.

Alignment is clearer: halign="right", valign="bottom" is translated automatically into
the number codes that represent the alignment.
"""



import ROOT
from array import array




class Legend( ROOT.TLegend ):
   def __init__( self, x1, y1, x2 = 1.1, y2 = 1.1, halign = "fixed", valign = "fixed", font=42, textSize = None ):
      """
         Either use like (x1,y1,x2,y2) or (x1,y1,halign="left",valign="top").
         If just (x1,y1) is used, halign="left" and valign="top" is assumed.
         
         Change the font with font=62 (default is 42).
      """
      if x2 == 1.1  and  halign == "fixed": halign = "left"
      if y2 == 1.1  and  valign == "fixed": valign = "top"
      self.halign = halign
      self.valign = valign
      ROOT.TLegend.__init__( self, x1, y1, x2, y2 )
      self.SetTextFont( font )
      if textSize: self.SetTextSize( textSize )

   def Draw( self ):
      # the coordinates in x1,y1 are always the corner the legend sticks to
      
      # need to set a fixed font size for the functions below to work
      if self.halign != "fixed" or self.valign != "fixed":
         if self.GetTextSize() < 0.0001:
            self.SetTextSize( 0.04 )
      
      # valign
      height = 1.3*self.GetTextSize()*self.GetNRows()
      if self.valign == "top":
         self.SetY2( self.GetY1() )
         self.SetY1( self.GetY2() - height )
      if self.valign == "bottom":
         self.SetY2( self.GetY1() + height )
      if self.valign == "center":
         center = self.GetY1()
         self.SetY2( center + height/2 )
         self.SetY1( center - height/2 )

      # halign
      width = 0.15 + self.GetTextSize()*self.GetNColumns()
      if self.halign == "left":
         self.SetX2( self.GetX1() + width )
      if self.halign == "right":
         self.SetX2( self.GetX1() )
         self.SetX1( self.GetX2() - width )
         print( "=--------------> "+str(width) )
      if self.halign == "center":
         center = self.GetX1()
         self.SetX1( center - width/2 )
         self.SetX2( center + width/2 )

      self.SetFillStyle( 0 )
      self.SetBorderSize( 0 )
      ROOT.TLegend.Draw( self )


class Graph( ROOT.TGraph ):
   def __init__( self, x, y=None, fillColor=None, lineColor=None, lineStyle=None, lineWidth=None, sort=True ):
      """ takes inputs of the form:
             x = [ (x1,y1), (x2,y2), ... ]
             y = None (default)
          or
             x = [x1,x2,...]
             y = [y1,y2,...]
      """

      if x == None:
         print( "WARNING: Tried to make graph of NULL object. Abort." )
         return

      if isinstance( x,ROOT.TObject ):
         ROOT.TGraph.__init__( self, x )
      
      else:
         if not y:
            # assume x is of the form: [ (x1,y1), (x2,y2), ... ]
            # --> split into xy
            y = [i[1] for i in x]
            x = [i[0] for i in x]
      
         if len(x) != len(y):
            print( "x and y have to have the same length." )
            return
            
         # sort
         if sort:
            xy = sorted( zip(x,y) )
            x = [i for i,j in xy]
            y = [j for i,j in xy]
   
         ROOT.TGraph.__init__( self, len(x), array('f',x), array('f',y) )
      
      
      if fillColor:
         self.SetFillColor( fillColor )
      if lineColor:
         self.SetLineColor( lineColor )
      if lineStyle:
         self.SetLineStyle( lineStyle )
      if lineWidth:
         self.SetLineWidth( lineWidth )
         
   def GetRanges( self ):
      r = ( ROOT.Double(), ROOT.Double(), ROOT.Double(), ROOT.Double() )
      self.ComputeRange( r[0], r[1], r[2], r[3] )
      return r

   def scale( self, factor ):
      for i in range( 0, self.GetN() ):
         p = ( ROOT.Double(), ROOT.Double() )
         self.GetPoint( i, p[0], p[1] )
         self.SetPoint( i, p[0], p[1]*factor )

   def add( self, term ):
      for i in range( 0, self.GetN() ):
         p = ( ROOT.Double(), ROOT.Double() )
         self.GetPoint( i, p[0], p[1] )
         self.SetPoint( i, p[0], p[1]+term )
   
   def integral( self ):
      """ Calculate integral using trapezoidal rule. """
      integral = 0.0
      for i in range( 1, self.GetN() ):
         previousPoint = ( ROOT.Double(), ROOT.Double() )
         thisPoint = ( ROOT.Double(), ROOT.Double() )
         self.GetPoint( i-1, previousPoint[0], previousPoint[1] )
         self.GetPoint( i, thisPoint[0], thisPoint[1] )
         
         integral += (thisPoint[0]-previousPoint[0]) * (thisPoint[1]+previousPoint[1])/2.0
      return integral
      
   def argminX( self ):
      """ Get the minimum X. """
      min = 1e30
      minX = None
      for i in range( 0, self.GetN() ):
         p = ( ROOT.Double(), ROOT.Double() )
         self.GetPoint( i, p[0], p[1] )
         if p[1] < min:
            min = p[1]
            minX = p[0]
      return minX
      
   def argminY( self ):
      """ Get the minimum Y. """
      min = 1e30
      for i in range( 0, self.GetN() ):
         p = ( ROOT.Double(), ROOT.Double() )
         self.GetPoint( i, p[0], p[1] )
         if p[1] < min: min = p[1]
      return min
      

   def table( self, bandLow=None, bandHigh=None, bandDifference=True ):
      out = ""
      
      for i in range( self.GetN() ):
         out += "%f \t%f" % ( self.GetX()[i], self.GetY()[i] )
         if bandLow:
            bL = bandLow.Eval( self.GetX()[i] )
            if bandDifference: bL -= self.GetY()[i]
            out += " \t"+str( bL )
         if bandHigh:
            bH = bandHigh.Eval( self.GetX()[i] )
            if bandDifference: bH -= self.GetY()[i]
            out += " \t"+str( bH )
         
         out += "\n"
         
      return out
         
   def getFirstIntersectionsWithGraph( self, otherGraph, xVar=None, xCenter=None, xRange=None, steps=1000 ):
      """ xRange must be of the form (min,max) when given """
      if xVar and not xRange: xRange = (xVar.getMin(), xVar.getMax())
      if xVar and not xCenter: xCenter = xVar.getVal()
      if not xRange: xRange = (self.GetRanges()[0], self.GetRanges()[2])
      
      low,high = (None,None)

      #down
      higher = self.Eval( xCenter ) > otherGraph.Eval( xCenter )
      for i in range( steps+1 ):
         #x = xRange[0]  +   float(i)*( xRange[1]-xRange[0] ) / steps
         x = xCenter  -   float(i)*( xCenter-xRange[0] ) / steps
         
         newHigher = self.Eval( x ) > otherGraph.Eval( x )
         if higher != newHigher:
            low = x
            break
         higher = newHigher
      #up
      higher = self.Eval( xCenter ) > otherGraph.Eval( xCenter )
      for i in range( steps+1 ):
         #x = xRange[0]  +   float(i)*( xRange[1]-xRange[0] ) / steps
         x = xCenter  +   float(i)*( xRange[1]-xCenter ) / steps
         
         newHigher = self.Eval( x ) > otherGraph.Eval( x )
         if higher != newHigher:
            high = x
            break
         higher = newHigher
         
      return (low,high)
      
   def getFirstIntersectionsWithValue( self, value, xVar=None, xCenter=None, xRange=None, steps=1000 ):
      """ xRange must be of the form (min,max) when given """
      if xVar and not xRange: xRange = (xVar.getMin(), xVar.getMax())
      if xVar and not xCenter: xCenter = xVar.getVal()
      if not xRange: xRange = (self.GetRanges()[0], self.GetRanges()[2])
      
      low,high = (None,None)

      #down
      higher = self.Eval( xCenter ) > value
      for i in range( steps+1 ):
         #x = xRange[0]  +   float(i)*( xRange[1]-xRange[0] ) / steps
         x = xCenter  -   float(i)*( xCenter-xRange[0] ) / steps
         
         newHigher = self.Eval( x ) > value
         if higher != newHigher:
            low = x
            break
         higher = newHigher         
      #up
      higher = self.Eval( xCenter ) > value
      for i in range( steps+1 ):
         #x = xRange[0]  +   float(i)*( xRange[1]-xRange[0] ) / steps
         x = xCenter  +   float(i)*( xRange[1]-xCenter ) / steps
         
         newHigher = self.Eval( x ) > value
         if higher != newHigher:
            high = x
            break
         higher = newHigher
         
      return (low,high)
      
   def getLatexIntervalFromNll( self, minX, up=0.5, xRange=None, steps=1000, digits=2 ):
      """ The parameter up is the same as in a Minos scan (0.5 for nll 
      and 68% two sided intervals). """
      
      mInterval = self.getFirstIntersectionsWithValue( up, xCenter=minX, xRange=xRange, steps=steps )
      fF = "%."+str(digits)+"f"   # float Format
      return ( (fF+"^{+"+fF+"}_{"+fF+"}") % (minX,mInterval[1]-minX,mInterval[0]-minX) )
      
      


class Band( ROOT.TGraph ):
   def __init__( self, x, yLow, yHigh, style="full", fillColor=None, lineColor=None, lineStyle=None, lineWidth=None, shiftBand=None ):
      """Possible styles: full, upperEdge, lowerEdge"""
      
      if style not in ["full", "upperEdge", "lowerEdge"]:
         print( "Style unknown. Using \"full\"." )
         style = "full"
      if len(x) != len(yLow) or len(x) != len(yHigh):
         print( "x, yLow and yHigh have to have the same length." )
         return
         
      if shiftBand:
         yLow = [y+s for y,s in zip(yLow,shiftBand)]
         yHigh = [y+s for y,s in zip(yHigh,shiftBand)]
         
      if style=="full":
         band_values =  sorted([(v[0],v[1]) for v in zip(x,yLow)])
         band_values += sorted([(v[0],v[1]) for v in zip(x,yHigh)], reverse=True)
         ROOT.TGraph.__init__( self, len(band_values), array('f',[v[0] for v in band_values]), array('f',[v[1] for v in band_values]) )
         self.SetLineWidth(0)
         
      if style=="upperEdge":
         band_values = [(v[0],v[1]) for v in zip(x,yHigh)]
         ROOT.TGraph.__init__( self, len(band_values), array('f',[v[0] for v in band_values]), array('f',[v[1] for v in band_values]) )
      
      if style=="lowerEdge":
         band_values = [(v[0],v[1]) for v in zip(x,yLow)]
         ROOT.TGraph.__init__( self, len(band_values), array('f',[v[0] for v in band_values]), array('f',[v[1] for v in band_values]) )

      if fillColor:
         self.SetFillColor( fillColor )
      if lineColor:
         self.SetLineColor( lineColor )
      if lineStyle:
         self.SetLineStyle( lineStyle )
      if lineWidth:
         self.SetLineWidth( lineWidth )
      
      

def DrawLine( x1,y1,x2,y2, lineWidth=None, lineStyle=None, lineColor=None, NDC=False ):
   l = ROOT.TLine( x1,y1,x2,y2 )
   if lineWidth: l.SetLineWidth( lineWidth )
   if lineStyle: l.SetLineStyle( lineStyle )
   if lineColor: l.SetLineColor( lineColor )
   
   if NDC:
      l.DrawLineNDC( x1,y1,x2,y2 )
   else:
      l.Draw()
   
   return l

def DrawHLine( y, lineWidth=None, lineStyle=None, lineColor=None ):
   ROOT.gPad.Update()
   x1,y1,x2,y2 = ( ROOT.Double(),ROOT.Double(),ROOT.Double(),ROOT.Double() )
   ROOT.gPad.GetRangeAxis( x1,y1, x2,y2 )
   return DrawLine(
      x1,y, x2,y,
      lineWidth, lineStyle, lineColor,
   )

def DrawVLine( x, lineWidth=None, lineStyle=None, lineColor=None ):
   ROOT.gPad.Update()
   x1,y1,x2,y2 = ( ROOT.Double(),ROOT.Double(),ROOT.Double(),ROOT.Double() )
   ROOT.gPad.GetRangeAxis( x1,y1, x2,y2 )
   return DrawLine(
      x,y1, x,y2,
      lineWidth, lineStyle, lineColor,
   )
   
def DrawBox( x1,y1, x2,y2, fillColor=None, lineColor=None, lineWidth=None, lineStyle=None ):
   b = ROOT.TBox( x1,y1, x2,y2 )
   if fillColor: b.SetFillColor( fillColor )
   if lineColor: b.SetLineColor( lineColor )
   if lineStyle: b.SetLineColor( lineStyle )
   if lineWidth: b.SetLineColor( lineWidth )
   b.Draw()
   return b


def DrawTextOneLine( x, y, text, textColor = 1, textSize = 0.04, NDC = True, halign = "left", valign = "bottom", skipLines = 0 ):
   """ This is just a helper. Don't use. Use DrawText instead. """
   
   halignMap = {"left":1, "center":2, "right":3}
   valignMap = {"bottom":1, "center":2, "top":3}
   
   scaleLineHeight = 1.0
   if valign == "top": scaleLineHeight = 0.8
   if skipLines: text = "#lower[%.1f]{%s}" % (skipLines*scaleLineHeight,text)
   
   # Draw the text quite simply:
   import ROOT
   l = ROOT.TLatex()
   if NDC: l.SetNDC()
   l.SetTextAlign( 10*halignMap[halign] + valignMap[valign] )
   l.SetTextColor( textColor )
   l.SetTextSize( textSize )
   l.DrawLatex( x, y, text )
   return l
   
def DrawText( x, y, text, textColor = 1, textSize = 0.04, NDC = True, halign = "left", valign = "bottom" ):
   objs = []
   skipLines = 0
   for line in text.split('\n'):
      objs.append( DrawTextOneLine( x, y, line, textColor, textSize, NDC, halign, valign, skipLines ) )
      if NDC == True: y -= 0.05 * textSize/0.04
      else:
         skipLines += 1
      
   return objs

def DrawTextAligned( x,y, textList, halignList=["right","center","left"], textColor = 1, textSize = 0.04, NDC = True, valign="bottom" ):
   """ use to write a = b and c = d under each other with the equal signs aligned """
   objs = []
   for text,halign in zip( textList, halignList ):
      objs.append( DrawText( x,y, text, textColor, textSize, NDC, halign, valign ) )
   return objs


