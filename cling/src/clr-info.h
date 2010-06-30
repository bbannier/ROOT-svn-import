
/* clr-info.h */

#ifndef __CLR_INFO_H__
#define __CLR_INFO_H__

#include "clang/AST/AST.h"
#include <string>

/* -------------------------------------------------------------------------- */

void info (const std::string msg, const std::string location = "");
void warning (const std::string msg, const std::string location = "");
void error (const std::string msg, const std::string location = "");

void init_trace ();

/* -------------------------------------------------------------------------- */

#endif /* __CLR_INFO_H__ */
