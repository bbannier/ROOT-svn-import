//
//  PadEditorController.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "CppWrapper.h"

#import "PadEditorController.h"


@implementation PadEditorController

- (void) tickActivated : (id) control
{
   PadParametersForEditor params = pad->GetPadParams();

   const unsigned on = [control isOn];
   if (control == tickX) {
      params.tickX = on;
   } else if (control == tickY) {
      params.tickY = on;
   }
   
   pad->SetPadParams(params);
   [padView setNeedsDisplay];
}

- (void) gridActivated : (id) control
{
   PadParametersForEditor params = pad->GetPadParams();

   const unsigned on = [control isOn];
   if (control == gridX) {
      params.gridX = on;
   } else if (control == gridY) {
      params.gridY = on;
   }
   
   pad->SetPadParams(params);
   [padView setNeedsDisplay];
}

- (IBAction) logActivated : (id) control
{
   PadParametersForEditor params = pad->GetPadParams();
   const unsigned on = [control isOn];
   
   if (control == logX)
      params.logX = on;
   
   if (control == logY)
      params.logY = on;
      
   if (control == logZ)
      params.logZ = on;
      
   pad->SetPadParams(params);
   [padView setNeedsDisplay];
}

- (void) hideEditor
{
   // First create a CATransition object to describe the transition
   CATransition *transition = [CATransition animation];
   // Animate over 3/4 of a second
   transition.duration = 0.75;
   // using the ease in/out timing function
   transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
   // Now to set the type of transition.
   transition.type = kCATransitionPush;
   transition.subtype = kCATransitionFromLeft;

   self.view.hidden = !self.view.hidden;
   [self.view.layer addAnimation:transition forKey:nil];
}

- (void) setView : (PadView *) view andPad : (PadWrapper *) newPad
{
   padView = view;
   pad = newPad;
   
   const PadParametersForEditor params = pad->GetPadParams();
   gridX.on = params.gridX;
   gridY.on = params.gridY;
   tickX.on = params.tickX;
   tickY.on = params.tickY;
   
   logX.on = params.logX;
   logY.on = params.logY;
   logZ.on = params.logZ;
}

@end
