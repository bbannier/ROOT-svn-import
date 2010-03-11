/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseOperation.h 30397 2009-09-24 13:08:16Z axel $
// author:  Alexei Svitkine

#if 0
#include "ParseEnvironment.h"

#include <clang/Basic/Version.h>
#include <llvm/System/Path.h>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/LexDiagnostic.h>
#include <clang/Lex/Preprocessor.h>
#include "clang/Frontend/InitPreprocessor.h"
#include <clang/Frontend/InitHeaderSearch.h>
#include <clang/AST/ASTContext.h>

namespace cling {
   
   //
   // ParseEnvironment
   //
   
   ParseEnvironment::ParseEnvironment(const clang::LangOptions& options,
                                      clang::TargetInfo& target,
                                      clang::Diagnostic *diag,
                                      clang::FileManager *fileMgr,
                                      clang::SourceManager *srcMgr,
                                      const std::vector<std::string>* inclPath,
                                      clang::PPCallbacks *callbacks) :
	m_srcMgr(srcMgr==0 ? new clang::SourceManager : 0),
   m_ext_srcMgr(srcMgr),
	m_fileMgr(fileMgr ? 0 : new clang::FileManager), // create a file manager only if we have to
	m_headerInfo(new clang::HeaderSearch(fileMgr ? *fileMgr : *m_fileMgr))
   {
      //------------------------------------------------------------------------
      // Fill the header database
      //------------------------------------------------------------------------
      
      clang::InitHeaderSearch hiInit(*m_headerInfo);
      hiInit.AddDefaultEnvVarPaths(options);
      hiInit.AddDefaultSystemIncludePaths(options, target.getTriple());
      llvm::sys::Path clangIncl(LLVM_LIBDIR, strlen(LLVM_LIBDIR));
      clangIncl.appendComponent("clang");
      clangIncl.appendComponent(CLANG_VERSION_STRING);
      clangIncl.appendComponent("include");
      hiInit.AddPath( clangIncl.c_str(), clang::InitHeaderSearch::System,
                     true, false, false, true /*ignore sysroot*/);
      if (inclPath) {
         for (std::vector<std::string>::const_iterator iP = inclPath->begin(),
                 iE = inclPath->end(); iP != iE; ++iP) {
            hiInit.AddPath(*iP, clang::InitHeaderSearch::Angled,
                           true, true, false);
         }
      }
      hiInit.Realize();
      
      //-------------------------------------------------------------------------
      // Create the preprocessor
      //-------------------------------------------------------------------------      
      m_preproc.reset(new clang::Preprocessor(*diag, options, target, *getSourceManager(), *m_headerInfo));
      m_preproc->setPPCallbacks(callbacks);
      clang::PreprocessorInitOptions ppOptions;
      InitializePreprocessor(*m_preproc, ppOptions);
      m_preproc->getBuiltinInfo().InitializeBuiltins(m_preproc->getIdentifierTable(),
                                                     m_preproc->getLangOptions().NoBuiltin);
      
      
      
      m_ast.reset(new clang::ASTContext(options,
                                       *getSourceManager(),
                                       target,
                                       m_preproc->getIdentifierTable(),
                                       m_preproc->getSelectorTable(),
                                       m_preproc->getBuiltinInfo()));
   }
   
   clang::ASTContext * ParseEnvironment::getASTContext() const
   {
      return m_ast.get();
   }
   
   clang::Preprocessor * ParseEnvironment::getPreprocessor() const
   {
      return m_preproc.get();
   }
   
   clang::SourceManager * ParseEnvironment::getSourceManager() const
   {
      return m_ext_srcMgr ? m_ext_srcMgr : m_srcMgr.get();
   }

}
#endif // 0
