#ifndef BASE__AST_H_
#define BASE__AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"

namespace ast
{
    namespace x3 = boost::spirit::x3;

    typedef boost::variant<std::string, int64_t, float, bool> Value;

    struct Var {
        std::string name;
        operator std::string() { return name; }
    };

    struct Element {
        Var var;
        std::string key;
    };

    struct Property {
        std::string key;
        Value value;
    };

    enum class EdgeDirection { right, left };

    struct Edge {
        Var var;
        std::vector<std::string> labels;
        std::vector<Property> properties;
        EdgeDirection direction;
    };

    struct Node {
        Var var;
        std::vector<std::string> labels;
        std::vector<Property> properties;
    };

    struct StepPath {
        Edge edge;
        Node node;
    };

    struct LinearPattern {
        // GraphId graph_id;
        Node root;
        std::vector<StepPath> path;
        std::string graph_name;
    };

    struct EQ { };

    struct NE { };

    struct GT { };

    struct LT { };

    struct GE { };

    struct LE { };

    typedef boost::variant<EQ, NE, GT, LT, GE, LE> Comparator;

    // struct valueWrap {
    //     boost::variant<element, value> value_
    // }

    struct Statement {
        boost::variant<Var, Element> lhs;
        Comparator comparator;
        boost::variant<Var, Element, Value> rhs;
    };

    struct Formula;

    struct And { };
    struct Or { };

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

    struct All { };

    struct Root {
        bool explain;
        boost::variant<All, std::vector<Element>> selection;
        std::vector<LinearPattern> graph_pattern;
        boost::optional<Formula> where;
        boost::optional<int> limit;
    };
}

#endif // BASE__AST_H_
