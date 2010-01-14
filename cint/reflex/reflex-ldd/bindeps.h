//==================================
// PEDUMP - Matt Pietrek 1994-2001
// FILE: EXEDUMP.CPP
//==================================

#ifndef BINDEPS_H
#define BINDEPS_H

#include <windows.h>
#include <vector>
#include <string>

void CollectDependentDlls( PIMAGE_DOS_HEADER dosHeader, std::vector<std::string>& result);

#endif // BINDEPS_H
