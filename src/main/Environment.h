#pragma once
#include <string>
#include <unordered_map>

#include "MALtypes.h"


using std::unordered_map;  using std::string;

class Environment : public std::enable_shared_from_this<Environment>{
public:
	~Environment() = default;

	//factory function because c'tor is private, so there's no way to have getptr return nullptr
	[[nodiscard]] static envPtr createPtr();
	[[nodiscard]] static envPtr createPtr(envPtr _outer);

	envPtr getPtr();

	mValPtr get(string key);
	void set(string key, mValPtr value);
	void setList(mValPtr bindings, mValPtr expressions);
	void remove(string key);
	bool contains(string key);
	envPtr find(string key);
	unordered_map<string, mValPtr>& getEnv();
private:
	Environment() = default;
	Environment(envPtr _outer);
	//TODO add a constructor that takes a (variable length) list of pairs of mValPtrs, and adds them to the environment


	unordered_map<string, mValPtr> env;
	envPtr outer;
};
