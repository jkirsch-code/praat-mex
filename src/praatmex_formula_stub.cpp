/*
 * praatmex_formula_stub.cpp
 *
 * Minimal stubs for Formula_run and Formula_compile.
 * The real Formula.cpp pulls in the entire scripting engine (Interpreter, ScriptEditor, etc.)
 * which is not needed for the headless MEX API.  These stubs satisfy the linker for the
 * few call sites that reference Formula_run (Table, Matrix, Sound_extensions, etc.).
 */

#include "melder.h"
#include "Formula.h"

void Formula_compile (Interpreter /* interpreter */, Daata /* data */, conststring32 /* expression */,
	int /* expressionType */, bool /* optimize */)
{
	Melder_throw (U"Formula compilation is not available in praatMEX.");
}

void Formula_run (integer /* row */, integer /* col */, Formula_Result * /* result */)
{
	Melder_throw (U"Formula evaluation is not available in praatMEX.");
}
