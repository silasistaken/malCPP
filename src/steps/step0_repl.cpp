#include "step0_repl.h"


namespace step0{

	string READ(string s){ return s; }
	string EVAL(string s){ return s; }
	string PRINT(string s){ return s; }
	string rep(string s){ return PRINT(EVAL(READ(s))); }

	int run(){
		string input;
		cout << "user> ";
		while(std::getline(std::cin, input)){
			try{
				string result = rep(input);
				cout << "output> ";
				cout << result << "\n";
			} catch(std::exception& e){
				cout << e.what();
			}
			cout << "user> ";
		}
		return 0;
	}
}