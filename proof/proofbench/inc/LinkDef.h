/* @(#)root/netx:$Id: LinkDef.h 20882 2007-11-19 11:31:26Z rdm $ */

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class Event+;
#pragma link C++ class EventHeader+;
#pragma link C++ class Track+;
#pragma link C++ class HistogramManager+;
//#pragma link C++ class TProofBench+;
#pragma link C++ class TProofNode+;
#pragma link C++ class TProofBenchRun+;
#pragma link C++ class TProofBenchRunDataRead+;
#pragma link C++ class TProofBenchRunCleanup+;
#pragma link C++ class TProofBenchRunCPU+;
#pragma link C++ class TProofBenchMode+;
#pragma link C++ class TProofBenchModeConstNFilesNode+;
#pragma link C++ class TProofBenchModeConstNFilesWorker+;
#pragma link C++ class TProofBenchModeVaryingNFilesWorker+;
#pragma link C++ class TProofBenchModeCleanup+;
#pragma link C++ class TProofBenchFileGenerator+;
#pragma link C++ class TSelHist+;
#pragma link C++ class TSelEvent+;
#pragma link C++ class TSelEventGen+;
#pragma link C++ class TSelEventGenN+;

#endif
