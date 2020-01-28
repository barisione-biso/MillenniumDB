#ifndef BASE_PARSING_EXCEPTIONS_H_
#define BASE_PARSING_EXCEPTIONS_H_

#include <exception>
#include <string>

struct ParsingException : public std::exception {
	const char * what () const throw () {
    	return "Parsing Exception";
    }
};

struct NotSupportedException : public std::exception {
	const char * what () const throw () {
    	return "Operation still not supported\n";
    }
};

// namespace ast {

//     struct VisitError
//         : public std::exception
//     {
//         std::string state;
//     };

//     struct EntityError
//         : public ast::VisitError
//     {
//         std::string state;

//         EntityError(std::string var)
//             : state("Inconsistent entity of of ?" + var) {}

//         inline const char * what() const throw() {
//             return state.c_str();
//         }
//     };

//     struct TypeError
//         : public ast::VisitError
//     {
//         std::string state;

//         TypeError(std::string var)
//             : state("Inconsistent value type of ?" + var) {}

//         inline const char * what() const throw() {
//             return state.c_str();
//         }
//     };

//     struct NotSupportedError
//         : public ast::VisitError
//     {
//         std::string state;

//         NotSupportedError(std::string err)
//             : state(std::move(err)) {}

//         inline const char * what() const throw() {
//             return state.c_str();
//         }
//     };


//     struct SelectionError
//         : public ast::VisitError
//     {
//         std::string state;

//         SelectionError(std::string var)
//             : state("The variable ?" + var + " is not in any graph pattern.") {}

//         const char * what() const throw() {
//             return state.c_str();
//         }
//     }; // struct SelectionError

// } // namespace ast

#endif