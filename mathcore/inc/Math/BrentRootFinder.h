// @(#)root/mathcore:$Id$
// Authors: David Gonzalez Maline    01/2008 

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header for the RootFinder
// 
// Created by: David Gonzalez Maline  : Wed Jan 21 2008
// 

#include <Math/IFunction.h>

namespace ROOT {
namespace Math {

class BrentRootFinder {
public:
   ~BrentRootFinder();
   BrentRootFinder();
   BrentRootFinder(const IGenFunction &, bool copy=false);
   
   // Implementing VirtualIntegratorOneDim Interface
   void SetFunction (const IGenFunction &, bool copy=false);
   double Root(double xmin, double xmax) const;

protected:
   double MinimStep(int type, double &xmin, double &xmax, double fy) const;
   double MinimBrent(int type, double &xmin, double &xmax, double xmiddle, double fy, bool &ok) const;

protected:
   const IGenFunction* fFunction;
   bool fFunctionCopied;
   const int fNpx;
};

};
};
