// @(#)root/proofx:$Id$
// Author: G. Ganis Oct 2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchTypes
#define ROOT_TProofBenchTypes

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ProofBenchTypes                                                      //
// Enum types used by ProofBench and its selectors                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

enum EPBFileType {kPBFileNotSpecified=0,  //type not specified
                  kPBFileBenchmark,       //file for benchmark test
                  kPBFileCleanup};        //file for cleaning up memory between

enum EPBReadType {
   kPBReadNotSpecified=0,                    //read type not specified
   kPBReadFull=1,                            //read in a full event
   kPBReadOpt=2,                             //read in part of an event
   kPBReadNo=4,                              //do not read in event
   kPBReadAll=kPBReadFull | kPBReadOpt | kPBReadNo //read in a full event,
                                             // part of a event and no event
};

enum EPBHistType {
   kPBHistNotSpecified=0,                  //histogram type not specified
   kPBHist1D=1,                            //1-D histogram
   kPBHist2D=2,                            //2-D histogram
   kPBHist3D=4,                            //3-D histogram
   kPBHistAll=kPBHist1D | kPBHist2D | kPBHist3D  //1-D, 2-D and 3-D histograms
};

enum EPBCleanupType {
   kPBCleanupNotSpecified=0,               //clean-up type not specified
   kPBCleanupFileAdvise=1,                 //clean-up by system call
   kPBCleanupReadInFiles=2                 //clean-up by reading in dedicated
                                          //files
};

#endif
