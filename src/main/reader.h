#pragma once
#include <vector>
#include <string>
#include <memory>//unique_ptr
#include <unordered_map>//hash-map
#include "MALtypes.h" //dont need forwards declaration if we import the whole header



//class to store the tokens and the position (emulating an "iterator")
class Reader{
public:
	Reader();
	//~Reader(){};

	// look at (returns) the next token
	string& peek();

	// look at (returns) the next token and advance the position
	string& next();

	bool eof() const;


	//turns a user input string into a properly formated string for internal use and remove surrounding double quotes
	static string unsecape_str(const string& token);

	//entry point, calls tokenize() and then read_form()
	static mValPtr read_string(const string& user_input);

	//peeks at the first symbol of next the token, calls either read_list or read_atom. returns a static malType
	static mValPtr read_form(Reader& readerObj);

	//calls read_form until it sees closing paren ")"
	static mValPtr read_list(Reader& readerObj, string eos_token);

	//turns a token into a malType
	static mValPtr read_atom(Reader& readerObj);

	//expands a reader macro and returns the resulting list
	static mValPtr expand_reader_macro(Reader& readerObj, const string& macro);

	//splits a string into individual tokens and returns them as a vector
	static vector<string> tokenize(const string& input_str);

private:
	vector<string> tokens;
	size_t position = 0;

};

//TODO wrap these functions up as to not pollute global namespace (either in the reader class or a separate namespace



