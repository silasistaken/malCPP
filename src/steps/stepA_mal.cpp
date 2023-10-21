#include "stepA_mal.h"
#include "repl.h"
#include "reader.h"
#include "printer.h"



namespace stepA{
	envPtr GLOBAL_ENV = REPL::init_environment();
	string rep(string s){ return REPL::rep(s, GLOBAL_ENV); }
}