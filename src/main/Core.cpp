#include "Core.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "myExceptions.h"
#include "printer.h"
#include "reader.h"
#include "repl.h"

using std::ifstream; using std::ofstream;  using std::dynamic_pointer_cast;

bool Core::addAll(envPtr env){
	bool successfullyAdded = true;
	successfullyAdded &= addArithmetic(env);
	successfullyAdded &= addPrinters(env);
	successfullyAdded &= addComparators(env);
	successfullyAdded &= addListOps(env);
	successfullyAdded &= addFileOps(env);
	successfullyAdded &= addAtomOps(env);
	successfullyAdded &= addREPLOps(env);

	successfullyAdded &= addMacros(env);
	return successfullyAdded;
}

bool Core::addArithmetic(envPtr& env){

	auto add = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);

		if(!args) throw implException("wrong type of arguments (not a list) in addition");
		if(args->size() != 2) throw malException("wrong number of arguments in addition");

		//assert that args are ints
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to addition must be integers");

		return MAKE_INT(arg1->getData() + arg2->getData());
	};

	env->set("+", MAKE_FUNCTION(add));


	auto minus = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);

		if(!args) throw implException("wrong type of arguments (not a list) in subtraction");
		if(args->size() != 2) throw malException("wrong number of arguments in subtraction");

		//assert that args are ints
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to subtraction must be integers");

		return MAKE_INT(arg1->getData() - arg2->getData());
	};
	env->set("-", MAKE_FUNCTION(minus));

	auto multiply = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);

		if(!args) throw implException("wrong type of arguments (not a list) in multiplication");
		if(args->size() != 2) throw malException("wrong number of arguments in multiplication");

		//assert that args are ints
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to multiplication must be integers");

		return MAKE_INT(arg1->getData() * arg2->getData());

	};
	env->set("*", MAKE_FUNCTION(multiply));

	auto divide = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);

		if(!args) throw implException("wrong type of arguments (not a list) in division");
		if(args->size() != 2) throw malException("wrong number of arguments in division");

		//assert that args are ints
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto divisor = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(divisor->getData() == 0) throw malException("division by zero");
		if(!arg1 || !divisor) throw malException("arguments to division must be integers");

		return MAKE_INT(arg1->getData() / divisor->getData());
	};
	env->set("/", MAKE_FUNCTION(divide));

	return true;
}

bool Core::addPrinters(envPtr& env){

	// print_readably = true, joins the results with " " and returns the new string.
	auto pr_str = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("wrong type of arguments (not a list) in print");

		string result = "";
		for(auto& arg : *args){
			result += Printer::pr_str(arg) + " ";
		}
		if(args->size() > 0)
			result.pop_back(); //remove the last space

		return MAKE_STRING(result);
	};
	env->set("pr-str", MAKE_FUNCTION(pr_str));


	// print_readably = false, concatenates the results together ("" separator), and returns the new string.
	auto str = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("wrong type of arguments (not a list) in str");
		string result = "";
		for(auto& arg : *args){
			result += Printer::pr_str(arg, false);
		}
		return MAKE_STRING(result);
	};
	env->set("str", MAKE_FUNCTION(str));


	// print_readably = true, joins the results with " ", prints the string to the screen and then returns nil.
	auto prn = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("wrong type of arguments (not a list) in prn");

		string result = "";
		for(auto& arg : *args){
			result += Printer::pr_str(arg) + " ";
		}
		if(args->size() > 0)
			result.pop_back(); //remove the last space

		std::cout << result << "\n";
		return MAKE_CONST_NIL;
	};
	env->set("prn", MAKE_FUNCTION(prn));



	// print_readably = false, joins the results with " ", prints the string to the screen and then returns nil.
	auto println = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("wrong type of arguments (not a list) in println");
		string result = "";
		for(auto& arg : *args){
			result += Printer::pr_str(arg, false) + " ";
		}
		if(args->size() > 0)
			result.pop_back(); //remove the last space
		std::cout << result << "\n";
		return MAKE_CONST_NIL;
	};
	env->set("println", MAKE_FUNCTION(println));


	return true;
}

bool Core::addComparators(envPtr& env){
	auto eq = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in =");

		if(*args->getItem(0) == *args->getItem(1)) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("=", MAKE_FUNCTION(eq));


	auto less = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in <");
		//TODO add float support
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to < must be integers");
		if(arg1->getData() < arg2->getData()) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("<", MAKE_FUNCTION(less));


	auto greater = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in >");
		//TODO add float support
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to > must be integers");
		if(arg1->getData() > arg2->getData()) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set(">", MAKE_FUNCTION(greater));


	auto lessEq = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in <=");
		//TODO add float support
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to <= must be integers");
		if(arg1->getData() <= arg2->getData()) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("<=", MAKE_FUNCTION(lessEq));

	auto greaterEq = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in >=");
		//TODO add float support
		auto arg1 = std::dynamic_pointer_cast<mInt>(args->getItem(0));
		auto arg2 = std::dynamic_pointer_cast<mInt>(args->getItem(1));
		if(!arg1 || !arg2) throw malException("arguments to >= must be integers");
		if(arg1->getData() >= arg2->getData()) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set(">=", MAKE_FUNCTION(greaterEq));



	return true;
}

bool Core::addListOps(envPtr& env){
	auto list = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		//TODO do we need a new list here?
		return argList;
	};
	env->set("list", MAKE_FUNCTION(list));


	auto is_list = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in list?");
		if(std::dynamic_pointer_cast<mList>(args->getItem(0))) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("list?", MAKE_FUNCTION(is_list));


	auto is_empty = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in empty?");

		auto lst = std::dynamic_pointer_cast<mSequence>(args->getItem(0));
		if(!lst) throw malException("argument to empty? is not a list/vector");
		if(lst->size() == 0) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("empty?", MAKE_FUNCTION(is_empty));


	auto count = [](const mValPtr argList)->mValPtr{
		auto args = std::dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in count");
		auto lst = std::dynamic_pointer_cast<mSequence>(args->getItem(0));
		if(!lst) return MAKE_INT(0); //not a sequence, return 0 
		return MAKE_INT(lst->size());
	};
	env->set("count", MAKE_FUNCTION(count));

	return true;
}

bool Core::addMacros(envPtr& env){
	try{
		REPL::rep("(def! not (fn* (a) (if a false true)))", env);
		REPL::rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \"\nnil)\")))))", env);
	} catch(malException e){
		// TODO log error
		return false;
	}
	return true;
}

bool Core::addFileOps(envPtr& env){
	auto slurp = [](const mValPtr argList)->mValPtr{
		//reads a file into a string
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in slurp");
		auto arg1 = dynamic_pointer_cast<mString>(args->getItem(0));
		if(!arg1) throw malException("argument to slurp is not a string");
		std::ifstream t(arg1->getData());
		std::stringstream buffer;
		buffer << t.rdbuf();
		auto s = MAKE_STRING(buffer.str());
		t.close();
		if(t.is_open()) throw implException("failed to close file in slurp");
		return s;
	};
	env->set("slurp", MAKE_FUNCTION(slurp));

	return true;
}

bool Core::addAtomOps(envPtr& env){
	auto atom = [](const mValPtr argList)->mValPtr{
		//creates a new atom
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in atom");
		return MAKE_ATOM(args->getItem(0));
	};
	env->set("atom", MAKE_FUNCTION(atom));

	auto is_atom = [](const mValPtr argList)->mValPtr{
		//checks if the argument is an atom
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in atom?");
		if(dynamic_pointer_cast<mAtom>(args->getItem(0))) return MAKE_CONST_TRUE;
		else return MAKE_CONST_FALSE;
	};
	env->set("atom?", MAKE_FUNCTION(is_atom));

	auto deref = [](const mValPtr argList)->mValPtr{
		//returns the value the atom points/refers to
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in deref");
		auto arg1 = dynamic_pointer_cast<mAtom>(args->getItem(0));
		if(!arg1) throw malException("argument to deref is not an atom");
		return arg1->getData();
	};
	env->set("deref", MAKE_FUNCTION(deref));

	auto reset = [](const mValPtr argList)->mValPtr{
		//Takes an atom and a Mal value; the atom is modified to refer to the given Mal value. The Mal value is returned.
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 2) throw malException("wrong number of arguments in reset!");
		auto atom = dynamic_pointer_cast<mAtom>(args->getItem(0));
		if(!atom) throw malException("first argument to reset! is not an atom");
		atom->setData(args->getItem(1));
		return args->getItem(1);
	};
	env->set("reset!", MAKE_FUNCTION(reset));

	auto swap = [env](const mValPtr argList)->mValPtr{
		//Takes an atom, a function, and zero or more function arguments. The atom's value is modified to the result of applying the function with the atom's value as the first argument and the optionally given function arguments as the rest of the arguments. The new atom's value is returned. 
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() < 2) throw malException("wrong number of arguments in swap!");

		auto argAtom = dynamic_pointer_cast<mAtom>(args->getItem(0));
		if(!argAtom) throw malException("first argument to swap! is not an atom");

		auto argFun = dynamic_pointer_cast<mFunction>(args->getItem(1));
		if(!argFun) throw malException("second argument to swap! is not a function");

		auto argList2 = make_shared<vector<mValPtr>>();
		argList2->push_back(argFun); //add the function
		argList2->push_back(argAtom->getData()); //add the atom's value

		for(size_t i = 2; i < args->size(); i++){ //add the rest of the arguments
			argList2->push_back(args->getItem(i));
		}
		auto argList3 = MAKE_LIST(argList2);
		auto result = REPL::EVAL_TCO(argList3,env);
		argAtom->setData(result);
		return argAtom->getData();
	};
	env->set("swap!", MAKE_FUNCTION(swap));

	return true;
}

bool Core::addREPLOps(envPtr& env){
	auto read_string = [](const mValPtr argList)->mValPtr{
		//exposes the reader
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in read-string");

		auto arg1 = dynamic_pointer_cast<mString>(args->getItem(0));
		if(!arg1) throw malException("argument to read-string is not a string");
		return Reader::read_string(arg1->getData());
	};
	env->set("read-string", MAKE_FUNCTION(read_string));

	auto eval = [env](const mValPtr argList)->mValPtr{
		//evaluates an expression in the global environment
		auto args = dynamic_pointer_cast<mList>(argList);
		if(!args) throw implException("argList is not a list");
		if(args->size() != 1) throw malException("wrong number of arguments in eval");

		return REPL::EVAL_TCO(args->getFirst(), env);
	};
	env->set("eval", MAKE_FUNCTION(eval));
	return true;
}
