
/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseEnvironment.h 30397 2009-09-24 13:08:16Z axel $
// author:  Alexei Svitkine

#include "Diagnostics.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/LexDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace cling {

//
// DiagnosticsProvider
//

DiagnosticsProvider::DiagnosticsProvider(llvm::raw_os_ostream& out,
                                         const clang::LangOptions& opts)
	: _tdp(out, false, true, false, true, false)
	, _diag(this)
{
	_tdp.setLangOptions(&opts);
	_diag.setDiagnosticMapping(clang::diag::ext_implicit_function_decl,
	                           clang::diag::MAP_ERROR);
	_diag.setDiagnosticMapping(clang::diag::warn_unused_expr,
	                           clang::diag::MAP_IGNORE);
	_diag.setDiagnosticMapping(clang::diag::warn_missing_prototype,
	                           clang::diag::MAP_IGNORE);
	_diag.setDiagnosticMapping(clang::diag::pp_macro_not_used,
	                           clang::diag::MAP_IGNORE);
	_diag.setSuppressSystemWarnings(true);
}

void DiagnosticsProvider::HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
                                           const clang::DiagnosticInfo &Info)
{
	std::pair<clang::diag::kind, unsigned> record = std::make_pair(Info.getID(),
		Info.getLocation().getManager().getFileOffset(Info.getLocation()) - _offs);
	if (_memory.insert(record).second) {
		_tdp.HandleDiagnostic(DiagLevel, Info);
	}
}

void DiagnosticsProvider::setOffset(unsigned offset)
{
	_offs = offset;
}

clang::Diagnostic * DiagnosticsProvider::getDiagnostic()
{
	return &_diag;
}

//
// ProxyDiagnosticClient
//

ProxyDiagnosticClient::ProxyDiagnosticClient(clang::DiagnosticClient *DC)
	: _DC(DC)
{
}

void ProxyDiagnosticClient::HandleDiagnostic(
	clang::Diagnostic::Level DiagLevel,
	const clang::DiagnosticInfo &Info)
{
	if (DiagLevel == clang::Diagnostic::Error)
		_errors.insert(std::make_pair(Info.getID(), Info));

	if (_DC)
		_DC->HandleDiagnostic(DiagLevel, Info);
}

bool ProxyDiagnosticClient::hadError(clang::diag::kind Kind) const
{
	return _errors.find(Kind) != _errors.end();
}

bool ProxyDiagnosticClient::hadErrors() const
{
	return !_errors.empty();
}

} // namespace cling

