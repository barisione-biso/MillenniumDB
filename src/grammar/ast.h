#if !defined(AST_H)
#define AST_H

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/var/var_id.h"

#include <iostream>
#include <string>

namespace ast
{
    namespace x3 = boost::spirit::x3;

    typedef boost::variant<std::string, int, float, bool, VarId> value;

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

    struct eq_ {
        inline static const std::string str = "==";
    };
    struct neq_{
        inline static const std::string str = "!=";
    };
    struct gt_{
        inline static const std::string str = ">";
    };
    struct lt_ {
        inline static const std::string str = "<";
    };
    struct geq_{
        inline static const std::string str = ">=";
    };
    struct leq_ {
        inline static const std::string str = "<=";
    };

    typedef boost::variant<eq_> comparator;

    // struct valueWrap {
    //     boost::variant<element, value> value_
    // }

    struct statement {
        element lhs_;
        comparator comparator_;
        boost::variant<element, value> rhs_;
    };

    struct formula;

    struct and_ {
        inline static const std::string str = "AND";
    };
    struct or_ {
        inline static const std::string str = "OR";
    };

    typedef boost::variant<
        statement,
        boost::recursive_wrapper<formula>
    > condition; // TODO: entender bien esto

    struct step_formula {
        boost::variant<and_> op_;
        condition cond_;
    };

    struct formula {
        condition root_;
        std::vector<step_formula> path_;
    };

    struct all_ {
        inline static const std::string str = "<ALL>";
    };

    struct root {
        boost::variant<all_, std::vector<element>> selection_;
        std::vector<linear_pattern> graphPattern_;
        boost::optional<formula> where_;
    };
}

#endif
