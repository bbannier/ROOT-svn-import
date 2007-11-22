// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
#include <TEveUtil.h>
#include <TEveManager.h>

#include <Getline.h>

int main(int argc, char **argv)
{
  TEveUtil::SetupEnvironment();

  int r = TEveManager::SpawnGuiAndRun(argc, argv);
  Getlinem(kCleanUp, 0);
  return r;
}
