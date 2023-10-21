#include "step9_try.h"
#include "repl.h"
#include "reader.h"
#include "printer.h"



namespace step9{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}