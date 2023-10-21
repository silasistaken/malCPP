#include "step7_quote.h"
#include "repl.h"



namespace step7{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}