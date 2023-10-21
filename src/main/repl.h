#pragma once
//#include <memory>//unique_ptr
#include <string>

#include "Environment.h"
#include "MALtypes.h"



class REPL{
public:
	static void start();
	static void start(int argc, char** argv);
	static mValPtr READ(std::string& input);
	static mValPtr EVAL_TCO(mValPtr input, envPtr  env);
	static std::string PRINT(mValPtr input);
	static std::string rep(std::string input, envPtr env);

	static envPtr init_environment();

private:
	static mValPtr eval_ast_tco(mValPtr ast, envPtr  env);
};