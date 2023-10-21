#include "step3_env.h"
#include "replNonTCO.h"




namespace step3{
	envPtr GLOBAL_ENV = REPLnonTCO::init_environment();
	string rep(string s){ return REPLnonTCO::rep(s, GLOBAL_ENV); }
}
