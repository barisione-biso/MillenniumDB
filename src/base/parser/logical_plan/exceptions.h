#ifndef BASE__PARSING_EXCEPTIONS_H_
#define BASE__PARSING_EXCEPTIONS_H_

#include <exception>
#include <string>

class ParsingException : public std::exception {
	const char * what () const throw () {
    	return "Error parsing query";
    }
};

class QuerySemanticException : public std::exception {
	std::string error;

	const char * what () const throw () {
    	return error.c_str();
    }
public:
	QuerySemanticException(std::string operation) {
		error = "Bad query semantic: " + operation + ".";
	}
};

class NotSupportedException : public std::exception {
	std::string error;

	const char * what () const throw () {
    	return error.c_str();
    }
public:
	NotSupportedException(std::string operation) {
		error = "Operation " + operation + " not supported yet.";
	}
};

#endif // BASE__PARSING_EXCEPTIONS_H_
