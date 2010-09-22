//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: InputValidator.h 35384 2010-09-17 15:06:50Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/InputValidator.h"

#include "llvm/Support/MemoryBuffer.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"

#include "cling/Interpreter/Interpreter.h"

#include <stack>
#include <iostream>

cling::InputValidator::InputType
cling::InputValidator::analyzeInput(llvm::StringRef contextSource,
                                    llvm::StringRef line,
                                    int& indentLevel,
                                    std::vector<clang::FunctionDecl*> *fds)
{
   // Check if there is an explicitation continuation character.
   if (line.size() > 1 && line[line.size() - 2] == '\\') {
      indentLevel = 1;
      return Incomplete;
   }
   //
   //  Setup a compiler instance to work with.
   //
   clang::CompilerInstance* CI = m_Interpreter->getCI();
   if (!CI) {
      return Incomplete;
   }
   CI->createPreprocessor();
   llvm::MemoryBuffer* buffer =
      llvm::MemoryBuffer::getMemBufferCopy(line, "CLING");
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return Incomplete;
   }
   clang::Token lastTok;
   bool tokWasDo = false;
   int stackSize = analyzeTokens(CI->getPreprocessor(), lastTok,
                                 indentLevel, tokWasDo);

   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   if (stackSize < 0) {
      return TopLevel;
   }
   // tokWasDo is used for do { ... } while (...); loops
   if (
      !lastTok.is(clang::tok::semi) &&
      (
         !lastTok.is(clang::tok::r_brace) ||
         tokWasDo
      )
   ) {
      return Incomplete;
   }
   if (stackSize > 0) {
      return Incomplete;
   }
   CI = m_Interpreter->getCI();
   if (!CI) {
      return TopLevel;
   }
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   // Setting this ensures "foo();" is not a valid top-level declaration.
   //diag.setDiagnosticMapping(clang::diag::ext_missing_type_specifier,
   //                          clang::diag::MAP_ERROR);
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   //CI->createASTContext();
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
      PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
      PP.getSelectorTable(), PP.getBuiltinInfo(), 0));

   PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                          PP.getLangOptions().NoBuiltin);
   //std::string src = contextSource + buffer->getBuffer().str();
   struct : public clang::ASTConsumer {
      bool hadIncludedDecls;
      unsigned pos;
      unsigned maxPos;
      clang::SourceManager* sm;
      std::vector<clang::FunctionDecl*> fds;
      void HandleTopLevelDecl(clang::DeclGroupRef D) {
         for (
            clang::DeclGroupRef::iterator I = D.begin(), E = D.end();
            I != E;
            ++I
         ) {
            clang::FunctionDecl* FD = dyn_cast<clang::FunctionDecl>(*I);
            if (FD) {
               clang::SourceLocation Loc = FD->getTypeSpecStartLoc();
               if (!Loc.isValid()) {
                  continue;
               }
               if (sm->isFromMainFile(Loc)) {
                  unsigned offset =
                     sm->getFileOffset(sm->getInstantiationLoc(Loc));
                  if (offset >= pos) {
                     fds.push_back(FD);
                  }
               }
               else {
                  while (!sm->isFromMainFile(Loc)) {
                     const clang::SrcMgr::SLocEntry& Entry =
                        sm->getSLocEntry(
                           sm->getFileID(sm->getSpellingLoc(Loc)));
                     if (!Entry.isFile()) {
                        break;
                     }
                     Loc = Entry.getFile().getIncludeLoc();
                  }
                  unsigned offset = sm->getFileOffset(Loc);
                  if (offset >= pos) {
                     hadIncludedDecls = true;
                  }
               }
            }
         }
      }
   } consumer;
   consumer.hadIncludedDecls = false;
   consumer.pos = contextSource.size();
   consumer.maxPos = consumer.pos + buffer->getBuffer().size();
   consumer.sm = &CI->getSourceManager();
   buffer = llvm::MemoryBuffer::getMemBufferCopy(line, "CLING");
   if (!buffer) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   //CI->setASTConsumer(0);  // We may use the consumer below.
   //CI->setASTContext(0);  // We may use the consumer below.
   if (CI->hasPreprocessor()) {
      CI->getPreprocessor().EndSourceFile();
   }
   CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   CI->getDiagnosticClient().EndSourceFile();
#if 0
   if (
      CI->getDiagnostics().hadError(
         clang::diag::err_unterminated_block_comment)
   ) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return Incomplete;
   }
#endif // 0
   if (
      !CI->getDiagnostics().getNumErrors() &&
      (
         !consumer.fds.empty() ||
         consumer.hadIncludedDecls
      )
   ) {
      if (!consumer.fds.empty()) {
         fds->swap(consumer.fds);
      }
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   return Stmt;
}

//---------------------------------------------------------------------------
// Note: Used only by analyzeInput().
int cling::InputValidator::analyzeTokens(clang::Preprocessor& PP,
                                      clang::Token& lastTok, int& indentLevel,
                                      bool& tokWasDo)
{
   std::stack<std::pair<clang::Token, clang::Token> > S; // Tok, PrevTok
   indentLevel = 0;
   PP.EnterMainSourceFile();
   clang::Token Tok;
   PP.Lex(Tok);
   while (Tok.isNot(clang::tok::eof)) {
      if (Tok.is(clang::tok::l_square)) {
         S.push(std::make_pair(Tok, lastTok));
      }
      else if (Tok.is(clang::tok::l_paren)) {
         S.push(std::make_pair(Tok, lastTok));
      }
      else if (Tok.is(clang::tok::l_brace)) {
         S.push(std::make_pair(Tok, lastTok));
         indentLevel++;
      }
      else if (Tok.is(clang::tok::r_square)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_square)) {
            std::cout << "Unmatched [\n";
            return -1;
         }
         tokWasDo = false;
         S.pop();
      }
      else if (Tok.is(clang::tok::r_paren)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_paren)) {
            std::cout << "Unmatched (\n";
            return -1;
         }
         tokWasDo = false;
         S.pop();
      }
      else if (Tok.is(clang::tok::r_brace)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_brace)) {
            std::cout << "Unmatched {\n";
            return -1;
         }
         tokWasDo = S.top().second.is(clang::tok::kw_do);
         S.pop();
         indentLevel--;
      }
      lastTok = Tok;
      PP.Lex(Tok);
   }
   int result = S.size();
   // TODO: We need to properly account for indent-level for blocks that do not
   //       have braces... such as:
   //
   //       if (X)
   //         Y;
   //
   // TODO: Do-while without braces doesn't work, e.g.:
   //
   //       do
   //         foo();
   //       while (bar());
   //
   // Both of the above could be solved by some kind of rewriter-pass that would
   // insert implicit braces (or simply a more involved analysis).
   // Also try to match preprocessor conditionals...
   if (result == 0) {
      clang::Lexer Lexer(PP.getSourceManager().getMainFileID(),
                         PP.getSourceManager().getBuffer(
                            PP.getSourceManager().getMainFileID()),
                         PP.getSourceManager(), PP.getLangOptions());
      Lexer.LexFromRawLexer(Tok);
      while (Tok.isNot(clang::tok::eof)) {
         if (Tok.is(clang::tok::hash)) {
            Lexer.LexFromRawLexer(Tok);
            if (clang::IdentifierInfo *II = PP.LookUpIdentifierInfo(Tok)) {
               switch (II->getPPKeywordID()) {
                  case clang::tok::pp_if:
                  case clang::tok::pp_ifdef:
                  case clang::tok::pp_ifndef:
                     ++result;
                     break;
                  case clang::tok::pp_endif:
                     if (result == 0)
                        return -1; // Nesting error.
                     --result;
                     break;
                  default:
                     break;
               }
            }
         }
         Lexer.LexFromRawLexer(Tok);
      }
   }
   return result;
}

