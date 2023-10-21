#include "step8_macros.h"
#include "repl.h"
#include "reader.h"
#include "printer.h"



namespace step8{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}