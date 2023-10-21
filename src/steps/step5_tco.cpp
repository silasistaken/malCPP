#include "step5_tco.h"
#include "repl.h"
//#include "Environment.h"



namespace step5{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}