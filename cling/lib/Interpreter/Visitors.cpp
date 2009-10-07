/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseEnvironment.h 30604 2009-10-07 15:10:56Z pcanal $
// author:  Alexei Svitkine

#include "Visitors.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Sema/SemaDiagnostic.h>

using std::string;

namespace cling {

//
// StmtFinder
//

StmtFinder::StmtFinder(unsigned pos, const clang::SourceManager& sm)
	: m_pos(pos), m_srcMgr(sm), m_stmt(NULL)
{
}

StmtFinder::~StmtFinder()
{
}

void StmtFinder::VisitChildren(clang::Stmt *S)
{
	for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
	     I != E; ++I) {
		if (*I) {
			Visit(*I);
		}
	}
}

void StmtFinder::VisitStmt(clang::Stmt *S)
{
	clang::SourceLocation Loc = S->getLocStart();
	unsigned offs = m_srcMgr.getFileOffset(m_srcMgr.getInstantiationLoc(Loc));
	if (offs == m_pos) {
		m_stmt = S;
	}
}

clang::Stmt * StmtFinder::getStmt() const
{
	return m_stmt;
}

//
// StmtSplitter
//

StmtSplitter::StmtSplitter(const string& src,
                           const clang::SourceManager& sm,
                           const clang::LangOptions& options,
                           std::vector<clang::Stmt*> *stmts)
		: _src(src), m_srcMgr(sm), _options(options), m_vec_stmts(stmts)
{
}

StmtSplitter::~StmtSplitter()
{
}

void StmtSplitter::VisitChildren(clang::Stmt *S)
{
	for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
	     I != E; ++I) {
		if (*I) {
			Visit(*I);
		}
	}
}

void StmtSplitter::VisitStmt(clang::Stmt *S)
{
   fprintf(stderr,"visit stmt\n");
   S->dump();
	m_vec_stmts->push_back(S);
}

} // namespace ccons
