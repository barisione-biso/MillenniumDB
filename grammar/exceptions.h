#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

namespace ast {
    struct EntityError
        : public std::exception
    {
        inline const char * what() const throw() {
            return "Wrong variable assignation in MATCH statement";
        }
    };

     struct TypeError
        : public std::exception
    {
        std::string var;

        TypeError(std::string var)
            : var(var) {}

        inline const char * what() const throw() {
            return "Inconsistent value type in WHERE statement with MATCH statement";
        }
    };

    struct SelectionError
        : public std::exception
    {
        std::string var;

        SelectionError(std::string var)
            : var(var) {}
        
        const char * what() const throw() {
            return "A referenced variable is not in any MATCH statement";
        }
    }; // struct SelectionError

} // namespace ast

#endif