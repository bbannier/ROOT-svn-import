// RUN: cat %s | %cling
// XFAIL: *

// TODO: When we can dyn_cast the clients
//#include "clang/Basic/Diagnostic.h"
//#include "clang/Frontend/CompilerInstance.h"
//#include "clang/Frontend/VerifyDiagnosticsClient.h"

//#include "cling/Interpreter/Interpreter.h"


//clang::Diagnostic& Diags = gCling->getCI()->getDiagnostics();
//clang::DiagnosticClient* Client = new clang::VerifyDiagnosticsClient(Diags, Diags.takeClient());
//Diags.setClient(Client);

#include <string>
std::string s;
std::string s; // expected-error {{redefinition of 's'}} expected-note {{previous definition is here}}

.q
