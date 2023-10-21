#include "reader.h"

#include <string>
#include <vector>
#include <regex>

#include <iostream>

#include "myExceptions.h"

using std::string;  using std::vector; using std::regex; using std::cout;

Reader::Reader() :position(0){}

string& Reader::peek(){ return tokens[position]; }

string& Reader::next(){ return tokens[position++]; }

bool Reader::eof() const{ return position >= tokens.size(); }

mValPtr Reader::read_string(const string& user_input){
	Reader readerObj;
	readerObj.tokens = tokenize(user_input);

	if(readerObj.eof()) //no tokens, signal to REPL to ignore this input/line
		throw commentException();

	return read_form(readerObj);
}

mValPtr Reader::read_form(Reader& readerObj){
	if(readerObj.eof())
		throw implException("eof in read_form");

	string& token = readerObj.peek();
	if(token.empty())
		throw implException("empty token in read_form");
	else{
		/*
		* 'x	-> (quote x)
		* ~x	-> (unquote x)
		* `x	-> (quasiquote x)
		* ~@x	-> (splice-unquote x)
		* @x	-> (deref x)
		*/
		if(token == "(") //list
			return read_list(readerObj, ")");
		else if(token == "[") //vector
			return read_list(readerObj, "]");
		else if(token == "{") //hash-map
			return read_list(readerObj, "}");
		else if( //reader macros
				token == "'" ||
				token == "~" ||
				token == "`" ||
				token == "~@" ||
				token == "@"
				)
			return expand_reader_macro(readerObj, token);
		else
			return read_atom(readerObj);
	}
}


mValPtr Reader::read_list(Reader& readerObj, const string closing_token){
	readerObj.next();//advance position past opening token

	mValVecPtr list = std::make_shared<vector<mValPtr>>();

	//call read_form until we encounter a closing_token 
	while(!readerObj.eof()){
		const string& token = readerObj.peek();
		if(token == closing_token){
			readerObj.next(); //advance position past closing token

			if(closing_token == "]") //vector
				return MAKE_VECTOR(list);

			else if(closing_token == "}") //hash-map
				return MAKE_HASH_MAP(list);

			else //defaults to list
				return MAKE_LIST(list);
		}
		try{
			// read list/vector content
			list->push_back(std::move(read_form(readerObj)));
		} catch(const commentException& e){
			//ignore, should lead to an error somewhere else since the list is not closed
			//FIX this is not a good solution, we should be able to handle comments in read_form
			cout << e.print_error() << "\n";
		}
	}

	throw malException("(EOF in read_list) missing closing bracket \"" + closing_token + "\"");
}

mValPtr Reader::read_atom(Reader& readerObj){
	/**
	* token categories :
	* 1) int		(^[+-]?[0-9]+$)
	* 2) float		(^[+-]?[0-9]\.[0-9]+$)
	* 3) constant	(true|false|nil)
	* 4) comment	(;.*)
	* 5) string		(".*) needs checking for empty or non closed string
	* 6) keyword	(^:.*) a token starting with a colon : (can be followed by anything for now)
	* 7) symbols	(.*) everything not yet captured
	*/
	const string& token = readerObj.next(); //consumes a token
	regex atomRegexp(R"((^[+-]?[0-9]+$)|(^[+-]?[0-9]\.[0-9]+$)|(^true$|^false$|^nil$)|(^;.*$)|(^".*)|(^:.*)|(.*))", regex::ECMAScript);
	std::smatch matches;

	std::regex_search(token, matches, atomRegexp);
	if(!matches.empty()){
		for(size_t n = 1;n < matches.size();n++){
			if(matches[n].matched){
				switch(n){
					case 1://int
					{
						return MAKE_INT(stoi(token));
					}
					case 2://float
					{
						return MAKE_FLOAT(stof(token));
					}
					case 3://constant
					{
						return MAKE_CONST(token);
					}
					case 4://comment
					{
						//there should be no comments here, they should be handled by tokenizer
						throw implException("comment found in read_atom");
					}
					case 5://string
					{
						return MAKE_STRING(unsecape_str(token));
					}
					case 6://keyword
					{
						return MAKE_KEYWORD(token.substr(1, string::npos));
					}
					case 7://symbol
					{
						return MAKE_SYMBOL(token);
					}
					default:
					{
						throw implException("default case in read_atom, no regex match");
					}
				}
			}
		}
	}
	else
		throw implException("no regex match for token (atom)");

	throw implException("end of read_atom (how tf? no regex match)");
}

mValPtr Reader::expand_reader_macro(Reader& readerObj, const string& macro){
	mValVecPtr lst = std::make_shared<vector<mValPtr>>();
	//we turn this token into a symbol and pass the following token into the reader pipeline. the symbol and the return value get packet into a list and returned
	if(macro == "'")
		lst->push_back(std::move(MAKE_SYMBOL("quote")));
	else if(macro == "~")
		lst->push_back(std::move(MAKE_SYMBOL("unquote")));
	else if(macro == "`")
		lst->push_back(std::move(MAKE_SYMBOL("quasiquote")));
	else if(macro == "~@")
		lst->push_back(std::move(MAKE_SYMBOL("splice-unquote")));
	else if(macro == "@")
		lst->push_back(std::move(MAKE_SYMBOL("deref")));
	else
		throw implException("reader macro < " + macro + " > not recognized");

	readerObj.next();//skip to next token 
	lst->push_back(std::move(read_form(readerObj))); //could be a list or atom, let read_form handle that
	return MAKE_LIST(lst);
}



vector<string> Reader::tokenize(const string& input_str){
	/*
	* Regex groups:
	* [\s,]*			any number of LEADING whitespaces (as well as \n , \r , \t , \f , and \v) and/or commas (not captured)
	* ~@				matches both chars in this order
	* [\[\]{}()'`~^@]	matches any one of the special characters;
	*  "(?:[\\].|[^\\"])*"?  Starts capturing at a double-quote and stops at the next double-quote unless it was preceded by a backslash
			in which case it includes it until the next double-quote. It will also match unbalanced strings (no ending double-quote).
			- starts with " followed by either
			- \\.	an escaped backslash (user typed 1 backslash symbol) which needs to be escaped otherwise it would affect the following char
				OR
			- [^\\"]	any character that is NOT an escaped backslash followed by a double qoute

	* ;.*				comment, anything following ;is ignored, until the end of the line, \n is not matched by . so it will stop at the end of the line
	*
	* [^\s\[\]{}('"`,;)]+
	*					is a sequence of one or more non-special characters i.e. anything that is allowed as normal symbols (captures atoms)
	*					(if it was * instead of + then the emtpy string would be matched)
	*					this will not allow any of the special characters as part of the symbol e.g. foo;bar would be matched as symbol foo and comment bar
	*					Note:  the folloing characters ARE allowed as part of symbols:	~ ^ @
	*/

	vector<string> tokens;
	regex tokenRegexp(R"([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]+))", regex::ECMAScript);

	std::sregex_token_iterator tokenIter(input_str.begin(), input_str.end(), tokenRegexp, 1);//last arg must be 1 to ignore the leading whitespaces and commas
	std::sregex_token_iterator eos;// end of sequence (default constructor)

	while(tokenIter != eos){
		//ignore the comment token
		if(tokenIter->str()[0] != ';')
			tokens.push_back(tokenIter->str());
		tokenIter++;
	}
	return tokens;
};


/*
* checks if the string is properly formated and turns escaped characters into their actual string representation. e.g. the 2 char sequence \n is turned into the single char '\n'
* note: (currently?) permits the empty string
* throws errors if:
*		- a char other than \, " or n follows a backslash
*		- when the ending quotes are being escaped
*		- string is not closed
* returns the unescaped string without surrounding quotes
*/
string Reader::unsecape_str(const string& token){

	//detect unclosed strings (and empty tokens)
	if(token.size() <= 1 or token.back() != '"')
		throw malException("string has no closing double quotes (EOF)");

	string cleaned_str = "";
	for(size_t i = 1; i < token.length() - 1; i++){
		if(token[i] == '\\'){ // a single backslash typed by the user indicating that the next character is to be escaped

			if(i == token.length() - 2)
				// the last char inside the quotes is an unescaped backslash (otherwise we wouldnt get here)
				throw malException("string has no closing double quotes (EOF)");
			i++; //skip the backslash and look at the next char
			if(token[i] == 'n')
				cleaned_str += '\n';
			else if(token[i] == '"')
				cleaned_str += '\"';
			else if(token[i] == '\\')
				cleaned_str += '\\';
			else
				// (non escaped) backslash followed by neither of the above
				throw malException("cannot escape " + token.substr(i, 1));

		}
		else
			cleaned_str += token[i];
	}
	return cleaned_str;
};
