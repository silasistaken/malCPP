#pragma once
#include "Environment.h"
#include "MALtypes.h"


class Core{
public:
	Core() = delete;
	~Core() = delete;
	static bool addAll(envPtr);
	static bool addArithmetic(envPtr&);
	static bool addPrinters(envPtr&);
	static bool addComparators(envPtr&);
	static bool addListOps(envPtr&);
	static bool addMacros(envPtr&);
	static bool addFileOps(envPtr&);
	static bool addAtomOps(envPtr&);
	static bool addREPLOps(envPtr&);

};


