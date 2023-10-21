#include "step2_eval.h"
#include "replNonTCO.h"

namespace step2{

	envPtr GLOBAL_ENV = REPLnonTCO::init_environment();
	string rep(string s){ return REPLnonTCO::rep(s, GLOBAL_ENV); }


}
