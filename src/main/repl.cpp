#include "repl.h"

#include <iostream>

#include "myExceptions.h"
#include "reader.h"
#include "printer.h"
#include "Core.h"

using std::cout;


mValPtr REPL::READ(std::string& input){ return Reader::read_string(input); }

string REPL::PRINT(mValPtr input){ return Printer::pr_str(input); }

mValPtr REPL::EVAL_TCO(mValPtr input, envPtr env){
	// these might get changed between iterations 
	mValPtr curr_ast = input;
	envPtr curr_env = env;

	while(true){
		auto ast = dynamic_pointer_cast<mList>(curr_ast);
		if(!ast) return eval_ast_tco(curr_ast, curr_env); //ast is not a list

		if(ast->size() == 0) return curr_ast; //empty list

		//non empty list
		auto first = dynamic_pointer_cast<mSymbol>(ast->getItem(0));
		if(first){ //first element is a symbol, check for special forms
			if(first->getData() == "def!"){
				//(def! symbol value)
				if(ast->size() != 3) throw malException("def! must have 2 parameters");

				auto symbolPtr = dynamic_pointer_cast<mSymbol>(ast->getItem(1));
				if(!symbolPtr) throw malException("def! first parameter is not a symbol");

				auto evaluated = EVAL_TCO(ast->getItem(2), curr_env);
				curr_env->set(symbolPtr->getData(), evaluated);
				return evaluated;
			}
			else if(first->getData() == "let*"){
				//(let* (bindings) body)
				envPtr letEnv = Environment::createPtr(curr_env);

				auto bindings = dynamic_pointer_cast<mSequence>(ast->getItem(1));
				if(!bindings) throw malException("let* bindings is not a seq");
				if(bindings->size() % 2 != 0) throw malException("let* bindings must have an even number of elements");

				for(size_t i = 0; i < bindings->size(); i += 2){
					auto symbolPtr = dynamic_pointer_cast<mSymbol>(bindings->getItem(i));
					if(!symbolPtr) throw malException("let* binding is not a symbol");

					auto evaluated = EVAL_TCO(bindings->getItem(i + 1), letEnv);
					letEnv->set(symbolPtr->getData(), evaluated);
				}
				curr_ast = ast->getItem(2);
				curr_env = letEnv;
				continue;
			}
			else if(first->getData() == "do"){
				//(do expressions...)
				if(ast->size() == 1) return MAKE_CONST("nil");
				ast->pop_front(); //remove do
				curr_ast = ast->pop_back(); //set last element as ast for next iteration
				eval_ast_tco(ast, curr_env); //evaluate all but last element
				continue;
			}
			else if(first->getData() == "if"){
				//(if cond true-expr false-expr) or (if cond true-expr)
				if(ast->size() > 4 or ast->size() < 3) throw malException("if must have 3 or 4 parameters");
				auto cond = EVAL_TCO(ast->getItem(1), curr_env);

				if(*cond)
					curr_ast = ast->getItem(2);
				else if(!*cond and ast->size() == 4)
					curr_ast = ast->getItem(3);
				else
					return MAKE_CONST("nil");
				continue;
			}
			else if(first->getData() == "fn*"){
				//(fn* (params) body)
				if(ast->size() != 3) throw malException("fn* must have 2 parameters");
				auto params = dynamic_pointer_cast<mSequence>(ast->getItem(1));
				if(!params) throw malException("fn* params is not a seq");

				return MAKE_FN(params, ast->getItem(2), curr_env);
			}
		}
		//normal apply/eval
		auto evaluated = dynamic_pointer_cast<mList>(eval_ast_tco(ast, curr_env));
		if(!evaluated) throw implException("eval_ast_tco did not return a list");

		auto f = dynamic_pointer_cast<mFunction>(evaluated->pop_front()); //first element is the function, rest of evaluated is the arguments
		if(!f) throw implException("first element of evaluated ast is not a function");


		if(f->isCore()) //core function
			return f->apply(evaluated);

		else{ //user defined function
			curr_ast = f->getBody();
			//make new env with the function's env as the outer env
			curr_env = Environment::createPtr(f->getEnv());
			// map args to parameters of the function in the new env
			//populate localEnv with (parameters -> argList)
			auto parameters = f->getParams();
			curr_env->setList(parameters, evaluated);
		}

	}
}

string REPL::rep(std::string input, envPtr env){ return PRINT(EVAL_TCO(READ(input), env)); }




mValPtr REPL::eval_ast_tco(mValPtr ast, envPtr env){
	//FIX make scalable?
	if(auto symbolPtr = dynamic_pointer_cast<mSymbol>(ast)){//is a symbol
		return env->get(symbolPtr->getData());
	}
	else if(auto listPtr = dynamic_pointer_cast<mSequence>(ast)){//is a list/vector
		// here we dont have to distinguish between lists and vectors, just return the same type as the input
		mValVecPtr newList = make_shared<vector<mValPtr>>();
		for(auto& item : *listPtr)
			newList->push_back(EVAL_TCO(item, env));

		if(dynamic_pointer_cast<mList>(ast))	return MAKE_LIST(newList);
		else									return MAKE_VECTOR(newList);

	}
	else if(auto hashMapPtr = dynamic_pointer_cast<mHashMap>(ast)){//is a hash-map
		for(auto& key_val_pair : *hashMapPtr)
			key_val_pair.second = EVAL_TCO(key_val_pair.second, env);

		return hashMapPtr;

	}
	else //is not a symbol or list/vector/hashmap -> self evaluating
		return ast;
}




envPtr REPL::init_environment(){
	envPtr env = Environment::createPtr();
	if(!Core::addAll(env))
		cout << "failed to add core functions to environment\n";
	return env;
}


void REPL::start(){
	envPtr GLOBAL_ENV = init_environment();

	string prompt = "user> ";
	string user_input;
	std::cout << prompt;

	while(std::getline(std::cin, user_input)){//reads from the cmd line and stores it in user_input
		try{
			string result = rep(user_input, GLOBAL_ENV);
			std::cout << "output> ";
			std::cout << result << "\n";

		} catch(commentException&){
			//do nothing
		} catch(myException& e){
			//catch all custom exceptions
			std::cerr << e.print_error() << "\n";
		} catch(std::exception& e){
			//catch all standard exceptions
			std::cerr << e.what();
		}
		std::cout << prompt;
	}
}

void REPL::start(int argc, char** argv){
	envPtr GLOBAL_ENV = init_environment();

	if(argc > 1){
		//handle args
		try{
			REPL::rep("(load-file \"" + string(argv[1]) + "\")", GLOBAL_ENV);
			string argv_str = "(";
			if(argc >= 2){
				for(int i = 2; i < argc; ++i){
					argv_str += string(argv[i]) + " ";
				}
				argv_str.pop_back();
			}
			argv_str += ")";
			REPL::rep("(def! *ARGV* (read-string " + argv_str + "))", GLOBAL_ENV);
		} catch(commentException&){
			//do nothing
		} catch(myException& e){
			//catch all custom exceptions
			std::cerr << e.print_error() << "\n";
		} catch(std::exception& e){
			//catch all standard exceptions
			std::cerr << e.what();
		}
	}

	string prompt = "user> ";
	string user_input;
	std::cout << prompt;

	while(std::getline(std::cin, user_input)){//reads from the cmd line and stores it in user_input
		try{
			string result = rep(user_input, GLOBAL_ENV);
			std::cout << "output> ";
			std::cout << result << "\n";

		} catch(commentException&){
			//do nothing
		} catch(myException& e){
			//catch all custom exceptions
			std::cerr << e.print_error() << "\n";
		} catch(std::exception& e){
			//catch all standard exceptions
			std::cerr << e.what();
		}
		std::cout << prompt;
	}

}
