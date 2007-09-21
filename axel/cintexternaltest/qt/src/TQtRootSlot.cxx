// @(#)root/gt:$Id$
// Author: Valery Fine      18/01/2007

/****************************************************************************
** $Id$
**
** Copyright (C) 2007 by Valeri Fine. Brookhaven National Laboratory.
**                                    All rights reserved.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
*****************************************************************************/
///////////////////////////////////////////////////////////////////////////
//
// The TQRootSlot singleton class introduces the global SLOT to invoke
// the  ROOT command line from the GUI signals
// Optionally one can execute TApplication::Terminate method directly
//
// It provides a Qt slot to attach the the CINT C++ interpreter 
// to any Qt signal
// To execute any C++ statement from the GUI oen should connect 
// his/her Qt signal with the Qt slot of the global instance of this class
//
//  connect(GUI object, SIGNAL(const char *editedLine),TQtRootSlot::CintSlot(),SLOT(ProcessLine(const char*)))
//
//  To terminate the ROOT from QUI element connect the signal with the Terminate slot.
//  For example to terminate ROOT and Qt smootly do
//  
//  connect(qApp,SIGNAL(lastWindowClosed()),TQtRootSlot::CintSlot(),SLOT(TerminateAndQuit())
//
///////////////////////////////////////////////////////////////////////////

#include "TQtRootSlot.h"
#include "TROOT.h"
#include "TApplication.h"
#include <qapplication.h>

TQtRootSlot *TQtRootSlot::fgTQtRootSlot = 0;
//____________________________________________________
TQtRootSlot *TQtRootSlot::CintSlot()
{
   // create and return the singleton
   if (!fgTQtRootSlot) fgTQtRootSlot = new TQtRootSlot();
   return fgTQtRootSlot;
}
//____________________________________________________
void TQtRootSlot::ProcessLine(const char *command)
{
     // execute the arbitrary ROOT /CINt command via 
     // CINT C++ interpreter and emit the result
     int error;
     gROOT->ProcessLine(command,&error);
     emit Error(error);
}
//____________________________________________________
void TQtRootSlot::Terminate(int status)const
{
    // the dedicated slot to terminate the ROOT application
   if (gApplication) gApplication->Terminate(status);
}

//____________________________________________________
void TQtRootSlot::Terminate()const
{
    // the dedicated slot to terminate the ROOT application
   if (gApplication) gApplication->Terminate(0);
}

//____________________________________________________
void TQtRootSlot::TerminateAndQuit() const
{
    // the dedicated  slot to terminate the ROOT application
    // and quit the Qt Application if any

   Bool_t rtrm = kTRUE;
   if (gApplication) {
      rtrm = gApplication->ReturnFromRun();
      gApplication->SetReturnFromRun(kTRUE);
      gApplication->Terminate(0);
   }
   if (qApp) qApp->quit();
   else if (!rtrm && gApplication ) {
      gApplication->SetReturnFromRun(rtrm);
      gApplication->Terminate(0);
   }
}

//__________________________________________________________________
bool QConnectCint(const QObject * sender, const char * signal)
{
   // Connect the Qt signal to the "execute C++ statement" via CINT SLOT
   // The first parameter of the Qt signal must be "const char*"
   return
   QObject::connect(sender,signal
      ,TQtRootSlot::CintSlot(),SLOT(ProcessLine(const char*)));
}

//__________________________________________________________________
bool QConnectTerminate(const QObject * sender, const char * signal)
{
   // Connect the Qt signal to the "TApplication::Terminate" method 
   // Any extra parameters of the Qt signal are discarded
   return
   QObject::connect(sender,signal
      ,TQtRootSlot::CintSlot(),SLOT(Terminate()));
}
