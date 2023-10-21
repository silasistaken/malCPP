#pragma once
#include <memory>//unique_ptr
#include <string>

#include "Environment.h"
#include "MALtypes.h"

class REPLnonTCO{
public:
	static void start();
	static mValPtr READ(std::string& input);
	static mValPtr EVAL(mValPtr input, envPtr  env);
	static std::string PRINT(mValPtr input);
	static std::string rep(std::string input, envPtr env);

	static envPtr init_environment();

private:
	static mValPtr eval_ast(mValPtr ast, envPtr  env);
	static mValPtr eval_special_form(std::shared_ptr<mList> ast, envPtr env);
	static bool is_special_form(std::shared_ptr<mList> ast);
};

