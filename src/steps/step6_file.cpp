#include "step6_file.h"
#include "repl.h"



namespace step6{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}