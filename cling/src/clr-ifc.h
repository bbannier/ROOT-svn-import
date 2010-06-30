
/* clr-ifc.h */

#ifndef __CLR_IFC_H__
#define __CLR_IFC_H__

#include "clang/AST/AST.h"
#include <string>

/* -------------------------------------------------------------------------- */

void ClrInit (clang::ASTContext * C,
              std::string print_file_name,
              std::string style_file_name,
              std::string dict_file_name);

void ClrSend (clang::Decl* D);

void ClrOutput ();

void ClrTest ();

/* -------------------------------------------------------------------------- */

#endif /* __CLR_IFC_H__ */
