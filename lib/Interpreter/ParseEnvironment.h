/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id$
// author:  Alexei Svitkine

#ifndef CLING_PARSEENVIRONMENT_H
#define CLING_PARSEENVIRONMENT_H

#include <string>
#include <vector>

#include <llvm/ADT/OwningPtr.h>

namespace clang {
   class LangOptions;
   class TargetInfo;
   class Diagnostic;
   class ASTContext;
   class Preprocessor;
   class PPCallbacks;
   class SourceManager;
   class HeaderSearch;
   class FileManager;
   class Token;
} // namespace clang


//
// ParseEnvironment
// 

namespace cling {

   class ParseEnvironment {
      
   public:
      //FIXME: take a CompilerInstance instead! 
      ParseEnvironment(const clang::LangOptions& options,
                       clang::TargetInfo& target,
                       clang::Diagnostic *diag,
                       clang::FileManager *fileMgr = 0,
                       clang::SourceManager *srcMgr = 0,
                       const std::vector<std::string>* inclPath = 0,
                       clang::PPCallbacks *callbacks = 0);
      
      clang::ASTContext * getASTContext() const;
      clang::Preprocessor * getPreprocessor() const;
      clang::SourceManager * getSourceManager() const;
      
   private:
      
      llvm::OwningPtr<clang::SourceManager> m_srcMgr;     // Source manager we own
      clang::SourceManager*                 m_ext_srcMgr; // Source manager we do not own.
      llvm::OwningPtr<clang::FileManager>   m_fileMgr;
      llvm::OwningPtr<clang::HeaderSearch>  m_headerInfo;
      llvm::OwningPtr<clang::Preprocessor>  m_preproc;
      llvm::OwningPtr<clang::ASTContext>    m_ast;
      
   };
   
}

#endif // CLING_PARSEENVIRONMENT_H
