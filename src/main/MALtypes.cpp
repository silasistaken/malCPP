#include "MALtypes.h"

#include <iostream>

#include "myExceptions.h"


/* INTEGER CLASS */
mInt::mInt(int value) :data(value){};
mInt::~mInt(){};

size_t mInt::hash(){
	return std::hash<int>()(data);
}
bool mInt::operator==(const mType& other){
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mInt&>(other).data;
}

string mInt::toString(bool print_readably) const{ return std::to_string(data); }
int mInt::getData(){ return data; }
;// end of integer class



/* FLOAT CLASS */
mFloat::mFloat(float value) : data(value){}
mFloat::~mFloat(){}
size_t mFloat::hash(){ return std::hash<float>{}(data); }
bool mFloat::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mFloat&>(other).data;
}
string mFloat::toString(bool print_readably) const{ return std::to_string(data); }
float mFloat::getData(){ return data; }
;// end of float class



/* CONSTANT CLASS */
mConst::mConst(string value) :data(value){
	if(value != "nil" and value != "false" and value != "true") throw implException(value + " should not be a constant type");
	truthiness = value != "nil" and value != "false";
};
mConst::~mConst(){};

size_t mConst::hash(){ return std::hash<string>{}(data); }
bool mConst::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mConst&>(other).data;
}
mConst::operator bool() const{ return truthiness; } //only nil and false are falsey

string mConst::toString(bool print_readably) const{ return data; }
string mConst::getData(){ return data; }
;// end of constant class



/* SYMBOL CLASS */
mSymbol::mSymbol(string value) : data(value){}
mSymbol::~mSymbol(){}
size_t mSymbol::hash(){ return std::hash<string>{}(data); }
bool mSymbol::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mSymbol&>(other).data;
}

string mSymbol::toString(bool print_readably) const{ return data; }
string mSymbol::getData(){ return data; }
;// end of symbol class



/* STRING CLASS */
mString::mString(string value) :data(value){};
mString::~mString(){};

size_t mString::hash(){ return std::hash<string>{}(data); }
bool mString::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;

	return data == dynamic_cast<const mString&>(other).data;
}

string mString::toString(bool print_readably) const{
	//TODO add support for more escaped chars?
	if(print_readably){
		string escaped_string;
		escaped_string += '"';
		for(const char& c : data){
			if(c == '\n')
				escaped_string += "\\n";
			else if(c == '\\')
				escaped_string += "\\\\";
			else if(c == '"')
				escaped_string += "\\\"";
			else
				escaped_string += c;
		}
		escaped_string += '"';
		return escaped_string;
	}
	else
		return data;
}
string mString::getData(){ return data; }
;// end of string class



/* KEYWORD CLASS  */
mKeyword::mKeyword(string value) :data(value){};
mKeyword::~mKeyword(){};

size_t mKeyword::hash(){ return std::hash<string>{}(data); }
bool mKeyword::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mKeyword&>(other).data;
}

string mKeyword::toString(bool print_readably) const{ return ":" + data; }
string mKeyword::getData(){ return data; }
;// end of keyword class



/* SEQUENCE BASE CLASS */
mSequence::mSequence(mValVecPtr values) :data(values){};
mSequence::~mSequence(){}

size_t mSequence::size() const{ return data->size(); }
bool mSequence::empty() const{
	return data->size() == 0;
}

mValPtr mSequence::pop_front(){
	if(data->empty()) throw implException("Cannot pop from empty sequence");
	mValPtr front = (*data)[0];
	data->erase(data->begin());
	return front;
}
mValPtr mSequence::pop_back(){
	if(data->empty()) throw implException("Cannot pop from empty sequence");
	mValPtr back = getLast();
	data->pop_back();
	return back;
}

vector<mValPtr>::iterator mSequence::begin(){ return data->begin(); }
vector<mValPtr>::iterator mSequence::end(){ return data->end(); }

mValPtr mSequence::getItem(size_t i) const{ return (*data)[i]; }
mValPtr mSequence::getFirst() const{
	if(data->empty()) throw implException("Cannot getFirst from empty sequence");
	return (*data)[0];
}
mValPtr mSequence::getLast() const{
	if(data->empty()) throw implException("Cannot getLast from empty sequence");
	return (*data)[data->size() - 1];
}

mValVecPtr mSequence::getData() const{ return data; }
string mSequence::print_data(bool print_readably) const{
	if(data->empty())
		return "";

	string s = "";
	for(auto& item : (*data)){
		if(item == nullptr)
			s.append("nullptr");
		else
			s.append(item->toString(print_readably));
		s.append(" ");
	}
	s.pop_back();
	return s;
}

size_t mSequence::combinedHash(std::size_t seed){
	//https://stackoverflow.com/a/59388432
	for(size_t i = 0; i < data->size(); i++)
		seed ^= (*data)[i]->hash() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
};
bool mSequence::operator==(const mType& other){

	auto other_seq = dynamic_cast<const mSequence*>(&other);
	if(!other_seq)return false;

	if(size() != other_seq->size()) return false;
	for(size_t i = 0; i < this->size(); i++)
		if(!(*this->getItem(i) == *other_seq->getItem(i))) return false;
	return true;
}
; // end of sequence base class



/* LIST CLASS */
mList::mList(mValVecPtr values) : mSequence(values){};
mList::~mList(){};

string mList::toString(bool print_readably) const{ return "(" + print_data(print_readably) + ")"; }
size_t mList::hash(){ return combinedHash(17); }
;// end of list class



/* VECTOR CLASS  */
mVector::mVector(mValVecPtr values) :mSequence(values){};
mVector::~mVector(){}

string mVector::toString(bool print_readably) const{ return "[" + print_data(print_readably) + "]"; };
size_t mVector::hash(){ return combinedHash(31); }
;// end of vector class



/* HASH-MAP CLASS */
mHashMap::mHashMap(mValVecPtr argList){
	//check that argList is even
	if(argList->size() % 2 != 0)
		throw malException("uneven number of arguments passed to hash-map!");
	//Add key-value pairs to map
	for(size_t i = 0; i < argList->size(); i += 2)
		data.emplace((*argList)[i], (*argList)[i + 1]);
};

mHashMap::~mHashMap(){};

string mHashMap::toString(bool print_readably) const{
	//print m_HashMap
	if(data.size() == 0) return "{}";
	string s = "{";
	for(auto& pair : data){
		s.append(pair.first->toString(print_readably));
		s.append(" ");
		s.append(pair.second->toString(print_readably));
		s.append(" ");
	}
	s.pop_back();
	s.append("}");
	return s;
}

unordered_map<mValPtr, mValPtr, mHashMap::mHasher, mHashMap::mComparator>::iterator mHashMap::begin(){ return data.begin(); }

unordered_map<mValPtr, mValPtr, mHashMap::mHasher, mHashMap::mComparator>::iterator mHashMap::end(){ return data.end(); }
//TODO may need to change this to a more complex hash
// FIX this matches the hash of the string representation of the hash-map
size_t mHashMap::hash(){ return std::hash<string>{}(toString(true)); }

bool mHashMap::operator==(const mType& other){
	// if the types are different, return false
	if(typeid(*this) != typeid(other)) return false;
	return data == dynamic_cast<const mHashMap&>(other).data;
}

void mHashMap::add(mValPtr key, mValPtr value){ data.emplace(key, value); }
;// end of hash-map class



/* FUNCTION  CLASS */

mFunction::mFunction(function<mValPtr(mValPtr)> f) : func(f), is_core(true){}
mFunction::mFunction(shared_ptr<mSequence> params, mValPtr body, envPtr env) : params(params), body(body), env(env), is_core(false){}
mFunction::~mFunction(){}

//FIX how do we handle functions as hash keys?
size_t mFunction::hash(){ return size_t(); }
//FIX same as above
bool mFunction::operator==(const mType& other){ return false; }

string mFunction::toString(bool print_readably) const{ return "#<function>"; }

mValPtr mFunction::apply(mValPtr args){ return func(args); }
bool mFunction::isCore() const{ return is_core; }
envPtr mFunction::getEnv() const{ return env; }
shared_ptr<mSequence> mFunction::getParams() const{ return params; }
mValPtr mFunction::getBody() const{ return body; }
function<mValPtr(mValPtr)> mFunction::getFunc() const{ return func; }
;// end of function class




/* ATOM  CLASS */
mAtom::mAtom(mValPtr ptr) : data(ptr){}

size_t mAtom::hash(){ return size_t(); }

bool mAtom::operator==(const mType& other){ return false; }

string mAtom::toString(bool print_readably) const{ return "(atom " + data->toString(print_readably) + ")"; }

mValPtr mAtom::getData() const{ return data; }

void mAtom::setData(mValPtr val){ data = val; }
;// end of atom class