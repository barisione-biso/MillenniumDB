#if !defined(AST_H)
#define AST_H

#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3/operator/optional.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <iostream>
#include <string>

namespace ast 
{
    namespace x3 = boost::spirit::x3;

    typedef x3::variant<std::string, int, double, bool> value;

    struct element {
        std::string function; // If empty string then no function
        std::string variable;
        std::string key;
    };    

    struct property {
        std::string key_;
        value value_; 
    };

    struct edge {
        std::string variable_;
        std::vector<std::string> labels_;
        std::vector<property> properties_; 
        bool isright_; // Points right
    };

    struct node {
        std::string variable_;
        std::vector<std::string> labels_;
        std::vector<property> properties_; 
    };

     struct step_path {
        edge edge_;
        node node_;
    };

    struct linear_pattern {
        node root_;
        std::vector<step_path> path_;
        // edge root_, path_;
    };

    struct statement {
        element rhs;
        std::string comp;
        boost::variant<element, value> lhs;
    };


    // struct formula {
    //     x3::forward_ast<condition> lhs, rhs;
    //     std::string conector;
    // };

    // typedef x3::variant<statement, formula> condition;

    struct root {
        x3::variant<char, std::vector<element>> selection_;
        std::vector<linear_pattern> graphPattern_;
        // std::string graphPattern_;
    };
    
        // std::vector<linearPattern> graphPatterns;
        // boost::optional<condition> where;
}


#endif