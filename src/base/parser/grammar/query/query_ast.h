#ifndef BASE__QUERY_AST_H_
#define BASE__QUERY_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/parser/grammar/common/common_ast.h"

namespace query_ast {
    namespace x3 = boost::spirit::x3;
    using namespace ast;

    struct Element {
        Var var;
        std::string key;
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

#endif // BASE__QUERY_AST_H_
