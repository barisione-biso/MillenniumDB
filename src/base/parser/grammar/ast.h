#ifndef BASE__AST_H_
#define BASE__AST_H_

#include <iostream>
#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/var/var_id.h"

namespace ast
{
    namespace x3 = boost::spirit::x3;

    typedef boost::variant<std::string, int, float, bool, VarId> Value;

    struct Element {
        std::string function; // If empty string then no function
        std::string variable;
        std::string key;
    };

    struct Property {
        std::string key;
        Value value;
    };

    enum class EdgeDirection { right, left };

    struct Edge {
        std::string var;
        std::vector<std::string> labels;
        std::vector<Property> properties;
        EdgeDirection direction;
    };

    struct Node {
        std::string var;
        std::vector<std::string> labels;
        std::vector<Property> properties;
    };

    struct StepPath {
        Edge edge;
        Node node;
    };

    struct LinearPattern {
        Node root;
        std::vector<StepPath> path;
    };

    struct EQ {
        inline static const std::string str = "==";
    };

    struct NE {
        inline static const std::string str = "!=";
    };

    struct GT {
        inline static const std::string str = ">";
    };

    struct LT {
        inline static const std::string str = "<";
    };

    struct GE {
        inline static const std::string str = ">=";
    };

    struct LE {
        inline static const std::string str = "<=";
    };

    typedef boost::variant<EQ, NE, GT, LT, GE, LE> Comparator;

    // struct valueWrap {
    //     boost::variant<element, value> value_
    // }

    struct Statement {
        Element lhs;
        Comparator comparator;
        boost::variant<Element, Value> rhs;
    };

    struct Formula;

    struct And {
        inline static const std::string str = "AND";
    };
    struct Or {
        inline static const std::string str = "OR";
    };

    struct Condition {
        bool negation;
        boost::variant <Statement, boost::recursive_wrapper<Formula>> content;

        Condition()
            : negation(false) {}
    };

    struct StepFormula {
        boost::variant<And, Or> op;
        Condition condition;
    };

    struct Formula {
        Condition root;
        std::vector<StepFormula> path;
    };

    struct All {
        // inline static const std::string str = "<ALL>";
    };

    struct Root {
        boost::variant<All, std::vector<Element>> selection;
        std::vector<LinearPattern> graph_pattern;
        boost::optional<Formula> where;
    };
}

#endif // BASE__AST_H_
