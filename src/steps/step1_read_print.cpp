#include "step1_read_print.h"
#include "reader.h"


namespace step1{
	string READ(string s){ return  Reader::read_string(s)->toString(); }
	string EVAL(string s){ return s; }
	string PRINT(string s){ return s; }
	string rep(string s){ return PRINT(EVAL(READ(s))); }

}