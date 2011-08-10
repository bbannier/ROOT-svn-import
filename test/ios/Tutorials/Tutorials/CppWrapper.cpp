//
//  ObjCWrapper.cpp
//  Tutorials
//
//  Created by Timur Pocheptsov on 7/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//DEBUG
#include <iostream>
//
#include <stdexcept>
#include <cstddef>
#include <cstdlib>
#include <memory>

#include <CoreGraphics/CoreGraphics.h>


#include "ObjCWrapper.h"

#include "TFrame.h"

#include "TextOperations.h"
#include "QuartzPainter.h"
#include "PadProxy.h"

#include "CppWrapper.h"

//______________________________________________________________________________
FontManagerWrapper::FontManagerWrapper()
                      : fManager(new ROOT_iOS::FontManager())
{
   //
}

//______________________________________________________________________________
FontManagerWrapper::~FontManagerWrapper()
{
   //For auto_ptr's dtor.
}
   
//______________________________________________________________________________
PainterWrapper::PainterWrapper(ROOT_iOS::FontManager *fontManager)
                  : fPainter(new ROOT_iOS::Painter(*fontManager))
{
   //
}

//______________________________________________________________________________
PainterWrapper::~PainterWrapper()
{
   //For auto_ptr's dtor.
}

//______________________________________________________________________________
void PainterWrapper::SetContext(CGContextRef ctx)
{
   //
   fPainter->SetContext(ctx);
}

//______________________________________________________________________________
PadWrapper::PadWrapper(unsigned w, unsigned h, ROOT_iOS::Painter *painter, ROOT_iOS::FontManager *fontManager)
               : fPad(new ROOT_iOS::PadProxy(w, h, *painter, *fontManager)),
                 fPainter(painter)
{
   //
}

//______________________________________________________________________________
PadWrapper::~PadWrapper()
{
   //
}

//______________________________________________________________________________
void PadWrapper::cd()
{
   fPad->cd();
}

//______________________________________________________________________________
void PadWrapper::Paint()
{
   fPad->Paint();
}

//______________________________________________________________________________
void PadWrapper::Clear()
{
   fPad->Clear();
}

//______________________________________________________________________________
void PadWrapper::SelectObject(unsigned x, unsigned y)
{
//   fPad->SelectObjectInPad(x, y);
}

//______________________________________________________________________________
void PadWrapper::ExRot(Int_t ev, Int_t px, Int_t py)
{
   const Int_t eventType = ev == pwButton1Down ? kButton1Down : ev == pwButton1Motion ? kButton1Motion : kButton1Up;
   fPad->ExecuteRotateView(eventType, px, py);
}

//______________________________________________________________________________
void PadWrapper::Pick(Int_t px, Int_t py)
{
   if (!fPad->SelectionIsValid()) {
      InitSelectionBuffer();
   }
   
   fPad->Pick(px, py);
}

//______________________________________________________________________________
void PadWrapper::InitSelectionBuffer()
{
   //Create bitmap context.
   const CGRect rect = CGRectMake(0.f, 0.f, 640.f, 640.f);
   CGContextRef ctx = GetImageContext(rect);
   
   //Now draw into this context.
   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
      
   //IMPORTANT!!! Disable aa, to avoid 
   //"non-clear" colors.
   CGContextSetAllowsAntialiasing(ctx, 0);
   //
   CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextFillRect(ctx, rect);
   //
   fPainter->SetContext(ctx);
   fPad->PaintForSelection();
      
   CGImageRef im = GetImageFromContext();
   CopySelectionBuffer(im);
   //Here - fill selection buffer.
   FreeImage(im);
      
   CGContextSetAllowsAntialiasing(ctx, 1);
   // make image out of bitmap context
   EndImageContext();
}

//______________________________________________________________________________
void PadWrapper::CopySelectionBuffer(CGImageRef im)
{
// Get image width, height. We'll use the entire image.
	const std::size_t pixelsWide = CGImageGetWidth(im);
	const std::size_t pixelsHigh = CGImageGetHeight(im);
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	const int bitmapBytesPerRow = (pixelsWide * 4);
	const int bitmapByteCount = (bitmapBytesPerRow * pixelsHigh);
	
	// Use the generic RGB color space.
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

	if (!colorSpace)
      throw std::runtime_error("Error allocating color space");
	
   fBitmapData.assign(bitmapByteCount, 0);

	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits 
	// per component. Regardless of what the source image format is 
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
   CGContextRef context = CGBitmapContextCreate (&fBitmapData[0], pixelsWide, pixelsHigh, 8, // bits per component
                                                 bitmapBytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst);
	if (!context)
      throw std::runtime_error("Context not created!");
	
	// Make sure and release colorspace before returning
	CGColorSpaceRelease(colorSpace);
	//Has context now.
	const CGRect rect = {{0.f, 0.f}, {pixelsWide, pixelsHigh}}; 
	// Draw the image to the bitmap context. Once we draw, the memory 
	// allocated for the context for rendering will then contain the 
	// raw image data in the specified color space.
	CGContextDrawImage(context, rect, im); 
	
	unsigned char *data = (unsigned char *)CGBitmapContextGetData(context);
   fPad->SetSelectionBuffer(pixelsWide, pixelsHigh, data);
	// When finished, release the context
	CGContextRelease(context); 
}

//______________________________________________________________________________
bool PadWrapper::Selected() const
{
   return fPad->Selected();
}

//______________________________________________________________________________
void PadWrapper::PaintSelected() const
{
   fPad->PaintSelected();
}

//______________________________________________________________________________
void PadWrapper::PaintShadowForSelected() const
{
   fPad->PaintShadowForSelected();
}

//______________________________________________________________________________
PadParametersForEditor PadWrapper::GetPadParams() const
{
   PadParametersForEditor params = {};
   params.fillColor = fPad->GetFillColor();
   params.fillPattern = fPad->GetFillStyle();
   
   params.tickX = fPad->GetTickx();
   params.tickY = fPad->GetTicky();
   
   params.gridX = fPad->GetGridx();
   params.gridY = fPad->GetGridy();
   
   params.logX = fPad->GetLogx();
   params.logY = fPad->GetLogy();
   params.logZ = fPad->GetLogz();
   
   return params;
}

//______________________________________________________________________________
void PadWrapper::SetPadParams(PadParametersForEditor params)
{
   fPad->SetFillColor(params.fillColor);
   fPad->SetFillStyle(params.fillPattern);
   
   fPad->SetTickx(params.tickX);
   fPad->SetTicky(params.tickY);

   fPad->SetGridx(params.gridX);
   fPad->SetGridy(params.gridY);
   
   fPad->SetLogx(params.logX);
   fPad->SetLogy(params.logY);
   fPad->SetLogz(params.logZ);
}
