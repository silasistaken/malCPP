#include "myExceptions.h"

myException::myException(string msg) :error_msg(msg){}
myException::~myException(){}
;



malException::malException(string msg) :myException(msg){};
string malException::print_error() const{ return "MAL_ERROR: " + error_msg; }
string malException::what() const{ return print_error(); }
;


implException::implException(string msg) :myException(msg){};
string implException::print_error() const{ return "IMPL_ERROR: " + error_msg; }
string implException::what() const{ return print_error(); }
;

commentException::commentException() :myException("comment"){}
string commentException::print_error() const{return "COMMENT";}
string commentException::what() const{return print_error();}
