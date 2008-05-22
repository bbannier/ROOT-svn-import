// @(#)root/proofd:$Id$
// Author: G. Ganis May 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdLauncher
#define ROOT_XrdProofdLauncher

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdLauncher                                                    //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Interface to a generic session launcher                              //
// Used by XrdProofdProofServMgr.                                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class XrdProofdLauncher {

 public:
   XrdProofdLauncher() { }
   virtual ~XrdProofdLauncher() { }

   // This function takes care to start the session with the give arguments;
   // additional information may be given via 'opt'.
   // The function should return 0 on success, -1 otherwise.
   // The actual success state will be stated by a successful callback call
   virtual int Create(const char *opt, int argc, const char *argv[])  = 0;
};

#endif
