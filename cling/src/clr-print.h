
/* clr-print.h */

#ifndef __CLR_PRINT_H__
#define __CLR_PRINT_H__

#include <string>

#include <Reflex/Reflex.h>

/* pseudo code from Reflex dictionaty */

void ClrPrint (std::string file_name,
               std::string style_file_name = "",
               bool use_html = false);

std::string ClrPrintScope (Reflex::Scope scope,
                           std::string style_file_name = "");

#endif /* __CLR_PRINT_H__ */
