// RUN: cat %s | %cling -I%p
// Tests the ChainedConsumer's ability to recover from errors. .x produces 
// #include \"CannotDotX.h\" \n void wrapper() {CannotDotX();}, which causes
// a TagDecl to be passed trough the consumers. This TagDecl is caught twice by
// the ChainedConsumer and cached is the queue of incoming declaration twice.
// If we encounter error the ChainedConsumer shouldn't try to remove the 
// declaration twice and this test makes sure of that.

#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/VerifyDiagnosticsClient.h"

#include "cling/Interpreter/Interpreter.h"

clang::Diagnostic& Diags = gCling->getCI()->getDiagnostics();
clang::DiagnosticClient* Client = new clang::VerifyDiagnosticsClient(Diags, Diags.takeClient());
Diags.setClient(Client);

.x CannotDotX.h() // expected-error {{use of undeclared identifier 'CannotDotX'}} 
.x CannotDotX.h() // expected-error {{use of undeclared identifier 'CannotDotX'}}

.q
