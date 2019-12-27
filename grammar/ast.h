#if !defined(AST_H)
#define AST_H

#include <boost/variant.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3/operator/optional.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <iostream>
#include <string>

namespace ast 
{
    namespace x3 = boost::spirit::x3;

    typedef boost::variant<std::string, int, double, bool> value;

    struct element {
        std::string function_; // If empty string then no function
        std::string variable_;
        std::string key_;
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
    };

    struct eq_ {};
    struct neq_ {};
    struct gt_ {};
    struct lt_ {};
    struct geq_ {};
    struct leq_ {};

    typedef boost::variant<eq_, neq_, gt_, lt_, geq_, leq_> comparator;

    // struct valueWrap {
    //     boost::variant<element, value> value_
    // }

    struct statement {
        element lhs_;
        comparator comparator_;
        boost::variant<element, value> rhs_;
    };

    struct formula;

    struct and_ {};
    struct or_ {};

    typedef boost::variant<
        statement,
        boost::recursive_wrapper<formula>> condition;
    
    struct step_formula {
        boost::variant<and_, or_> op_;
        condition cond_;
    };
    
    struct formula {
        condition root_;
        std::vector<step_formula> path_;
    };

    struct all_ {};

    struct root {
        boost::variant<all_, std::vector<element>> selection_;
        std::vector<linear_pattern> graphPattern_;
        formula where_;
    };
}


#endif