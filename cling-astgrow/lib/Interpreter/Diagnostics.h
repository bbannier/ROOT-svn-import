#ifndef CLING_DIAGNOSTICS_H
#define CLING_DIAGNOSTICS_H

/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id$
// author:  Alexei Svitkine

#include <set>
#include <map>

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

namespace llvm {
   class raw_os_ostream;
}

namespace clang {
   class LangOptions;
}

namespace cling {
   
   //
   // DiagnosticsProvider provides a re-usable clang::Diagnostic object
   // that can be used for multiple parse operations.
   //
   
   class DiagnosticsProvider : public clang::DiagnosticClient {
      
   public:
      
      DiagnosticsProvider(llvm::raw_os_ostream& out, const clang::LangOptions& opts);
      
      void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
                            const clang::DiagnosticInfo &Info);
      
      void setOffset(unsigned offset);
      
      clang::Diagnostic * getDiagnostic();
      
   private:
      
      unsigned _offs;
      clang::TextDiagnosticPrinter _tdp;
      clang::Diagnostic _diag;
      std::set<std::pair<clang::diag::kind, unsigned> > _memory;
      
   };
   
   
   //
   // ProxyDiagnosticClient can act as a proxy to another diagnostic client.
   //
   
   class ProxyDiagnosticClient : public clang::DiagnosticClient {
      
   public:
      
      ProxyDiagnosticClient(clang::DiagnosticClient *DC);
      
      void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
                            const clang::DiagnosticInfo &Info);
      
      bool hadError(clang::diag::kind Kind) const;
      bool hadErrors() const;
      
   private:
      
      clang::DiagnosticClient *_DC;
      std::multimap<clang::diag::kind, const clang::DiagnosticInfo> _errors;
      
   };
   
   
} // namespace cling

#endif // CLING_DIAGNOSTICS_H
