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

    // TODO: for now only accepting kleene star, e.g. `:P123*`
    struct PropertyPath {
        std::string type;
        EdgeDirection direction;
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

    struct LinearPatternStep {
        boost::variant<Edge, PropertyPath> path;
        Node node;
    };

    struct LinearPattern {
        Node root;
        std::vector<LinearPatternStep> path;
    };

    struct GraphPattern {
        std::vector<LinearPattern> pattern;
        std::vector<boost::recursive_wrapper<GraphPattern>> optionals;
    };

    enum class BinaryOp {
        And,
        Or
    };

    enum class Order {
        Ascending,
        Descending
    };

    struct OrderedSelectItem {
        SelectItem item;
        Order order;
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

    struct Root {
        bool                                             explain;
        std::vector<SelectItem>                          selection;
        std::vector<LinearPattern>                       graph_pattern;
        boost::optional<Formula>                         where;
        boost::optional<std::vector<OrderedSelectItem>>  group_by;
        boost::optional<std::vector<OrderedSelectItem>>  order_by;
        boost::optional<uint_fast32_t>                   limit;
    };
}}

#endif // BASE__QUERY_AST_H_
