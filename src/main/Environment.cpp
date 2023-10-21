#include "Environment.h"

#include "myExceptions.h"


Environment::Environment(envPtr _outer) :outer(_outer){}

unordered_map<string, mValPtr>& Environment::getEnv(){ return env; }

// cant use make_shared because c'tor is private, this makes sure that every instance of Environment is owned by a shared_ptr
envPtr Environment::createPtr(){ return std::shared_ptr<Environment>(new Environment()); }

envPtr Environment::createPtr(envPtr _outer){ return std::shared_ptr<Environment>(new Environment(_outer)); }

//returns a pointer to the current object that shares ownership of the current object, never returns nullptr
envPtr Environment::getPtr(){ return shared_from_this(); }

mValPtr  Environment::get(string key){
	try{
		return	find(key)->getEnv()[key];
	} catch(malException& e){
		// string env = "{";
		// for(auto& pair : getEnv()){
		// 	env += pair.first + " : " + pair.second->toString() + ", ";
		// }
		// env.pop_back(); env.pop_back();
		// env += "}";
		// throw malException(e.what() + "\n" + env);
		throw e;
	}
}

//TODO return bool to indicate success
void Environment::set(string key, mValPtr  value){ env[key] = value; }

//TODO return bool to indicate success
//maps the keys in bindings to the values in expressions, takes variable length arguments denoted by &
void Environment::setList(mValPtr bindings, mValPtr expressions){
	auto keyList = dynamic_pointer_cast<mSequence>(bindings);
	if(!keyList) throw implException("env bindings must be a seq");
	auto valList = dynamic_pointer_cast<mSequence>(expressions);
	if(!valList) throw implException("env expressions must be a seq");

	//keyList may look like this: (a b c & d) or like this: (a b c) or like this: (& d) or like this: () 
	if(valList->size() == 0 and keyList->size() == 0) return;

	//TOODO check if there are too many args (not caught by &)
	for(size_t i = 0; i < keyList->size(); i++){
		auto symbolPtr = dynamic_pointer_cast<mSymbol>(keyList->getItem(i));
		if(!symbolPtr) throw malException("lambda binding is not a symbol");

		if(i >= valList->size() and symbolPtr->getData() != "&") throw malException("too few arguments");

		//valList must have at least as many items as keyList has before & 
		if(symbolPtr->getData() == "&"){ //rest parameter
			if(i != keyList->size() - 2) throw malException("rest parameter (&) must be second to last binding");

			auto rest_name = dynamic_pointer_cast<mSymbol>(keyList->getItem(i + 1));
			if(!rest_name) throw malException("rest parameter name must be a symbol");

			auto rest = MAKE_LIST(std::make_shared<vector<mValPtr>>(valList->begin() + i, valList->end()));
			set(rest_name->getData(), rest);
			break;
		}
		else
			set(symbolPtr->getData(), valList->getItem(i));
	}
}

void Environment::remove(string key){ env.erase(key); }

bool Environment::contains(string key){ return env.count(key) == 1; }

//TODO is this gonna be more useful in the future?
envPtr Environment::find(string key){
	if(contains(key))			return getPtr();
	else if(outer == nullptr)	throw malException("symbol " + key + " not found in environment");
	else						return outer->find(key);
}

