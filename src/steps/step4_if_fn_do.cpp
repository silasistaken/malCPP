#include "step4_if_fn_do.h"
#include "replNonTCO.h"

namespace step4
{
	envPtr GLOBAL_ENV = REPLnonTCO::init_environment();
	string rep(string s) { return REPLnonTCO::rep(s, GLOBAL_ENV); };
	// ugly hack to make the tests work, without changing Core.cpp to also handle REPLnonTCO
	string s = rep("(def! not (fn* (a) (if a false true)))");
	string t = rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \"\nnil)\")))))");
}
