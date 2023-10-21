#pragma once

#include <string>
using std::string;


//base class for all custom exceptions (is this really needed?)
class myException{
public:
	myException(string msg);
	virtual ~myException();
	virtual string print_error() const = 0;
	virtual string what() const = 0;
protected:
	string error_msg;
};



class malException :public myException{
public:
	malException(string msg);
	~malException() = default;
	string print_error() const override;
	string what() const override;
};



class implException :public myException{
public:
	implException(string msg) ;
	~implException() = default;
	string print_error() const override;
	string what() const override;
};



class commentException :public myException{
public:
	commentException();
	~commentException() = default;
	string print_error() const override;
	string what() const override;
};
