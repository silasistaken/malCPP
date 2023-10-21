#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

// #include "Environment.h"

using std::dynamic_pointer_cast;
using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

class mType;
class Environment;
using mValPtr = shared_ptr<mType>;
using envPtr = shared_ptr<Environment>;
using mValVecPtr = shared_ptr<vector<mValPtr>>;

// macros to make unique pointers for each data type
#define MAKE_INT(arg) make_shared<mInt>(arg)
#define MAKE_FLOAT(arg) make_shared<mFloat>(arg)
#define MAKE_CONST(arg) make_shared<mConst>(arg)
#define MAKE_CONST_NIL make_shared<mConst>("nil")
#define MAKE_CONST_TRUE make_shared<mConst>("true")
#define MAKE_CONST_FALSE make_shared<mConst>("false")
#define MAKE_SYMBOL(arg) make_shared<mSymbol>(arg)
#define MAKE_STRING(arg) make_shared<mString>(arg)
#define MAKE_KEYWORD(arg) make_shared<mKeyword>(arg)
#define MAKE_LIST(arg) make_shared<mList>(arg)
#define MAKE_VECTOR(arg) make_shared<mVector>(arg)
#define MAKE_HASH_MAP(arg) make_shared<mHashMap>(arg)
#define MAKE_FUNCTION(arg) make_shared<mFunction>(arg)
#define MAKE_FN(params, body, env) make_shared<mFunction>(params, body, env)
#define MAKE_ATOM(arg) make_shared<mAtom>(arg)

/**
 * this serves as a base class for all the datatypes(ints, floats, strings, lists etc.)
 * can maybe inherit from a wraper class that acts as a replace for a pointer
 * hopefully unique pointers are sufficient
 */
class mType
{
public:
	// make it so this class can be used as a key in a map
	virtual size_t hash() = 0;
	virtual bool operator==(const mType &other) = 0;
	virtual explicit operator bool() const { return true; } // only nil and false are falsey

	virtual ~mType() = default;
	virtual string toString(bool print_readably = true) const = 0;

	// TODO add a copy constructor and assignment operator for all types
};

class mInt : public mType
{
public:
	mInt(int value);
	~mInt();

	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;
	string toString(bool print_readably = true) const override;
	int getData();

private:
	int data;
};

class mFloat : public mType
{
public:
	mFloat(float value);
	~mFloat();

	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;
	float getData();

private:
	float data;
};

class mConst : public mType
{
public:
	mConst(string value);
	~mConst();
	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;
	explicit operator bool() const;

	string toString(bool print_readably = true) const override;
	string getData();

private:
	string data;
	bool truthiness;
};

/////////////////////////////////////////////
////////////// ATOM DATA TYPES //////////////
/////////////////////////////////////////////
class mSymbol : public mType
{
public:
	mSymbol(string value);
	~mSymbol();
	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;
	string getData();

private:
	string data;
};

// TODO maybe unite string and keyword
class mString : public mType
{
public:
	mString(string value);
	~mString();

	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;
	string getData();

private:
	string data;
};

// TODO store data without colon(:) and then add it back when printing
class mKeyword : public mType
{
	/*
	 * this is alot like symbols BUT a keyword always evaluates to itself.
	 * that means its just literally "just a word" and can be used as a name without the worry of it evalating to something unwanted. also safes a quote (i think)
	 */
public:
	mKeyword(string value);
	~mKeyword();
	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;
	string getData();

private:
	string data;
};

/////////////////////////////////////////////
/////////// SEQUENTIAL DATA TYPES ///////////
/////////////////////////////////////////////

// TODO consider datatype for list and vector based on expected use,
//  e.g. lists usually add to the front of it -> list (double) or forward-list (single linked)
//  vectors are indexed and prolly append to the back  -> vector (cpp)

// parent class for all sequential data types, holds the values and prints them (no brackets/parenthesis)
class mSequence : public mType
{
public:
	mSequence(mValVecPtr values);
	~mSequence();

	// TODO add functionality thats common to all seq types here
	string print_data(bool print_readably) const;

	size_t size() const;
	bool empty() const;

	mValPtr getItem(size_t i) const;
	mValPtr getFirst() const;
	mValPtr getLast() const;
	mValVecPtr getData() const; // TODO remove

	vector<mValPtr>::iterator begin();
	vector<mValPtr>::iterator end();

	mValPtr pop_front();
	mValPtr pop_back();

	// TODO add functions to add items to the front and back of the sequence, return a new (sub)sequence

	size_t combinedHash(std::size_t seed);

	bool operator==(const mType &other) override;

protected:
	mValVecPtr data; // shared_ptr to vector of mValPtrs
};

class mList : public mSequence
{
public:
	mList(mValVecPtr values);
	~mList();

	// implement hash and ==
	size_t hash() override;
	// bool operator==(const mType& other) override;

	string toString(bool print_readably = true) const override;
};

class mVector : public mSequence
{
public:
	mVector(mValVecPtr values);
	~mVector();

	// implement hash and ==
	size_t hash() override;
	// bool operator==(const mType& other) override;

	string toString(bool print_readably = true) const override;
};

// make a class that holds an unordered map that takes mValPtr as key and value and iherits from mType
class mHashMap : public mType
{

public:
	// constructor
	// mHashMap(std::unordered_map<mValPtr, mValPtr> map) : data(map){};
	mHashMap(mValVecPtr argList);
	~mHashMap();

	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;

	// add a key value pair to the map, if key already exists, this has no effect
	void add(mValPtr key, mValPtr value);

	// CUSTOM HASH FUNCTION AND COMPARATOR FOR mValPtr
	struct mHasher
	{
		size_t operator()(const mValPtr &key) const
		{
			return key->hash();
		}
	};
	struct mComparator
	{
		bool operator()(const mValPtr &lhs, const mValPtr &rhs) const
		{
			return *lhs == *rhs;
		}
	};

	// returns an iterator to the first element of the map
	unordered_map<mValPtr, mValPtr, mHasher, mComparator>::iterator begin();

	// returns an iterator to the last element of the map
	unordered_map<mValPtr, mValPtr, mHasher, mComparator>::iterator end();

private:
	// the underlying map
	unordered_map<mValPtr, mValPtr, mHasher, mComparator> data;
};

// a function that inherits from mType and can store a core functions or a user defined function
class mFunction : public mType
{
public:
	mFunction(function<mValPtr(mValPtr)> function);
	mFunction(shared_ptr<mSequence> params, mValPtr body, envPtr env);
	~mFunction();

	// implement hash and ==
	size_t hash() override;
	bool operator==(const mType &other) override;

	string toString(bool print_readably = true) const override;

	mValPtr apply(mValPtr args);

	// returns true if this is a core function
	bool isCore() const;

	// returns the environment of this function
	envPtr getEnv() const;

	// returns the parameters of this function
	shared_ptr<mSequence> getParams() const;

	// returns the body of this function
	mValPtr getBody() const;

	// returns the function object
	function<mValPtr(mValPtr)> getFunc() const;

private:
	bool is_core = false;
	function<mValPtr(mValPtr)> func;
	envPtr env;
	shared_ptr<mSequence> params;
	mValPtr body;
};

// this acts as a pointer to a mal value
class mAtom : public mType
{
public:
	mAtom(mValPtr ptr);
	~mAtom() = default;

	// Inherited via mType
	size_t hash() override;
	bool operator==(const mType &other) override;
	string toString(bool print_readably) const override;

	mValPtr getData() const;
	void setData(mValPtr val);

private:
	mValPtr data;
};