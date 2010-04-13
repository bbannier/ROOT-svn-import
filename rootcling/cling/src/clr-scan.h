
/* clr-scan.h */

#ifndef __CLR_SCAN_H__
#define __CLR_SCAN_H__

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclFriend.h"

#include <string>

/* ---------------------------------------------------------------------- */

extern const char * clang_key;

void ClrInit (clang::ASTContext * C,
              std::string print_file_name,
              std::string style_file_name,
              std::string dict_file_name);

void ClrScan (clang::Decl * D);

void ClrOutput ();

/* ---------------------------------------------------------------------- */

void ClrStore (clang::ASTContext * C,
               clang::Decl * D);

void ClrInfo ();

void ClrTest ();

/* ---------------------------------------------------------------------- */

typedef const char * const_char_ptr;

void ClrOptions (int & param_argc, const_char_ptr * & param_argv);

int driver_main (int argc, const_char_ptr * argv);

/* ---------------------------------------------------------------------- */

#endif /* __CLR_SCAN_H__ */
