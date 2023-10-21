#include "replNonTCO.h"

#include <iostream>

#include "MALtypes.h"
#include "Environment.h"
#include "myExceptions.h"
#include "reader.h"
#include "printer.h"
#include "Core.h"

using std::cout;


mValPtr REPLnonTCO::READ(std::string& input){ return Reader::read_string(input); }


mValPtr REPLnonTCO::EVAL(mValPtr input, envPtr  env){
	//TODO: if we need to check for list-ness, maybe give mType a virtual function isList() that returns a bool

	// here we have to distinguish between lists and vectors
	if(auto ast = dynamic_pointer_cast<mList>(input)){ //input is a LIST
		//empty list
		if(ast->size() == 0) return input;
		//non empty list
		else{
			//APPLY SECTION

			if(is_special_form(ast))
				return eval_special_form(ast, env);

			//normal apply/eval (first item is a function)
			auto evaluated_list = dynamic_pointer_cast<mList>(eval_ast(input, env));//call eval_ast to evaluate each item in the list
			if(!evaluated_list) throw implException("eval_ast returned a non-list from a list");

			if(evaluated_list->size() == 0) throw implException("eval_ast turned a non empty list into an empty list");

			auto func = dynamic_pointer_cast<mFunction>(evaluated_list->getItem(0));
			if(!func) throw malException("first item " + evaluated_list->getItem(0)->toString() + " is not a function or lambda");
			if(!func->isCore()) throw implException("tco function in non tco repl");
			evaluated_list->pop_front();//remove the function from the list
			return func->apply(evaluated_list); //apply the function to the rest of the list
		}
	}
	//input is not a list
	return eval_ast(input, env);
}

string REPLnonTCO::PRINT(mValPtr input){ return Printer::pr_str(input); }

string REPLnonTCO::rep(string input, envPtr env){ return PRINT(EVAL(READ(input), env)); }



mValPtr REPLnonTCO::eval_ast(mValPtr ast, envPtr env){
	//FIX  is this scalable? 
	if(auto symbolPtr = dynamic_pointer_cast<mSymbol>(ast)){//is a symbol
		return env->get(symbolPtr->getData());

	}
	else if(auto listPtr = dynamic_pointer_cast<mSequence>(ast)){//is a list/vector
		// here we dont have to distinguish between lists and vectors, just return the same type as the input
		mValVecPtr newList = make_shared<vector<mValPtr>>();
		for(auto& item : *listPtr)
			newList->push_back(EVAL(item, env));

		if(dynamic_pointer_cast<mList>(ast))	return MAKE_LIST(newList);
		else									return MAKE_VECTOR(newList);

	}
	else if(auto hashMapPtr = dynamic_pointer_cast<mHashMap>(ast)){//is a hash-map
		for(auto& key_val_pair : *hashMapPtr)
			// TODO maybe evaluate the keys too?
			key_val_pair.second = EVAL(key_val_pair.second, env);

		return hashMapPtr;

	}
	else //is not a symbol or list/vector/hashmap -> self evaluating
		return ast;
}

mValPtr REPLnonTCO::eval_special_form(shared_ptr<mList> ast, envPtr env){
	auto symbolPtr = dynamic_pointer_cast<mSymbol>(ast->getItem(0));
	if(!symbolPtr) throw malException("special form first item is not a symbol"); //this should be caught in is_special_form()


	if(symbolPtr->getData() == "def!"){
		//(def! symbol value)
		if(ast->size() != 3) throw malException("def! must have 2 parameters");

		auto firstItem = dynamic_pointer_cast<mSymbol>(ast->getItem(1));
		if(!firstItem) throw malException("def! first parameter is not a symbol");

		auto evaluated = EVAL(ast->getItem(2), env);
		env->set(firstItem->getData(), evaluated);
		return evaluated;

	}
	else if(symbolPtr->getData() == "let*"){
		//(let* (params) body)
		envPtr letEnv = Environment::createPtr(env);

		auto bindings = dynamic_pointer_cast<mSequence>(ast->getItem(1));
		if(!bindings) throw malException("let* bindings is not a list");

		for(size_t i = 0; i < bindings->size(); i += 2){
			auto binding = dynamic_pointer_cast<mSymbol>(bindings->getItem(i));
			if(!binding) throw malException("let* binding is not a symbol");

			auto evaluated = EVAL(bindings->getItem(i + 1), letEnv);
			letEnv->set(binding->getData(), evaluated);
		}
		return EVAL(ast->getItem(2), letEnv);

	}
	else if(symbolPtr->getData() == "do"){
		//(do expressions...)
		//eval all expressions in order, return last (or nil if empty) 
		if(ast->size() == 1) return MAKE_CONST("nil");
		ast->pop_front();
		mValPtr result = eval_ast(ast, env);
		if(auto res_lst = dynamic_pointer_cast<mList>(result))
			return res_lst->getLast();
		else throw implException("do: eval_ast returned a non-list");

	}
	else if(symbolPtr->getData() == "if"){
		//(if cond true-expr false-expr)
		if(ast->size() > 4 or ast->size() < 3) throw malException("if must have 3 or 4 parameters");
		auto cond = EVAL(ast->getItem(1), env);

		if(*cond)
			return EVAL(ast->getItem(2), env);
		else if(!*cond and ast->size() == 4)
			return EVAL(ast->getItem(3), env);
		else
			return MAKE_CONST("nil");
	}
	else if(symbolPtr->getData() == "fn*"){ //aka lambda
		//(fn* (parameters) body)
		//(fn* (parameters & more) body)
		// the (local) parameters get mapped to the parameters passed to the lambda call using a (local) environment.

		if(ast->size() != 3) throw malException("fn* must have 2 parameters");
		auto params = dynamic_pointer_cast<mSequence>(ast->getItem(1));
		if(!params) throw malException("fn* parameters is not a list");

		auto body = ast->getItem(2);

		auto closure = [env, params, body](const mValPtr argList)->mValPtr{
			auto argListPtr = dynamic_pointer_cast<mSequence>(argList);
			if(!argListPtr) throw malException("lambda arg list is not a list");

			auto localEnv = Environment::createPtr(env);

			//populate localEnv with (parameters -> argList)
			//for(auto i = 0; i < params->size(); i++){
			//	if(i > argListPtr->size()) throw malException("lambda called with too few arguments");
			//	auto symbolPtr = dynamic_pointer_cast<mSymbol>(params->getItem(i));
			//	if(!symbolPtr) throw malException("lambda binding is not a symbol");

			//	if(symbolPtr->getData() == "&"){ //rest parameter
			//		if(i != params->size() - 2) throw malException("lambda rest parameter must be second to last binding");

			//		auto rest_name = dynamic_pointer_cast<mSymbol>(params->getItem(i + 1));
			//		auto rest = MAKE_LIST(make_shared<vector<mValPtr>>(argListPtr->begin() + i, argListPtr->end()));
			//		localEnv->set(rest_name->getData(), rest);
			//		break;
			//	}
			//	else
			//		localEnv->set(symbolPtr->getData(), argListPtr->getItem(i));
			//}
			localEnv->setList(params, argListPtr);

			return EVAL(body, localEnv);
		};

		//return make_shared<mFunction>(closure);
		return MAKE_FUNCTION(closure);
	}

	return ast;
}

bool REPLnonTCO::is_special_form(shared_ptr<mList> ast){
	auto symbolPtr = dynamic_pointer_cast<mSymbol>(ast->getItem(0));
	if(!symbolPtr) return false; //first item is not a symbol
	if(symbolPtr->getData() == "let*"
	   || symbolPtr->getData() == "def!"
	   || symbolPtr->getData() == "do"
	   || symbolPtr->getData() == "fn*"
	   || symbolPtr->getData() == "if"
	   ) return true;
	else return false;
}



envPtr REPLnonTCO::init_environment(){
	envPtr env = Environment::createPtr();
	if(!Core::addAll(env))
		cout << "failed to add core functions to environment\n";
	return env;
}

void REPLnonTCO::start(){
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
