#ifndef BASE__QUERY_AST_H_
#define BASE__QUERY_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/parser/grammar/common/common_ast.h"

namespace query { namespace ast {
    namespace x3 = boost::spirit::x3;
    using namespace common::ast;

    struct SelectItem {
        std::string var;
        boost::optional<std::string> key;
    };

    struct Edge {
        std::string var_or_id; // Can be the identifier, a variable or empty
        std::vector<std::string> types;
        std::vector<Property> properties;
        EdgeDirection direction;
    };

    struct Node {
        std::string var_or_id; // Can be the identifier, a variable or empty
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

    enum class BinaryOp {
        And,
        Or
    };

    enum class Comparator {
        EQ, // ==
        NE, // !=
        GT, // >
        LT, // <
        GE, // >=
        LE, // <=
    };

    struct Statement {
        SelectItem lhs;
        Comparator comparator;
        boost::variant<SelectItem, ast::Value> rhs;
    };

    struct Formula;

    struct Condition {
        bool negation;
        boost::variant <Statement, boost::recursive_wrapper<Formula>> content;

        Condition() :
            negation(false) { }
    };

    struct StepFormula {
        BinaryOp op;
        Condition condition;
    };

    struct Formula {
        Condition root;
        std::vector<StepFormula> path;
    };

    struct QueryRoot {
        bool                           explain;
        std::vector<SelectItem>        selection;
        std::vector<LinearPattern>     graph_pattern;
        boost::optional<Formula>       where;
        boost::optional<uint_fast32_t> limit;
    };
}}

#endif // BASE__QUERY_AST_H_
