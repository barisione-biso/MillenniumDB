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

    enum class BinaryOp {
        And,
        Or
    };

    enum class Order {
        Ascending,
        Descending
    };

    struct SelectItem {
        std::string var;
        boost::optional<std::string> key;
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

    enum class PropertyPathSuffix {
        NONE,
        ZERO_OR_MORE, // *
        ONE_OR_MORE,  // +
        ZERO_OR_ONE   // ?
    };

    struct PropertyPathBoundSuffix {
        uint_fast32_t min;
        uint_fast32_t max;
    };

    struct PropertyPathAtom; // TODO: change name?

    // using PropertyPathSequence     = std::vector<PropertyPathAtom>;
    // using PropertyPathAlternatives = std::vector<PropertyPathSequence>;
    struct PropertyPathSequence {
        std::vector<PropertyPathAtom> atoms;
    };

    struct PropertyPathAlternatives {
        std::vector<PropertyPathSequence> alternatives;
    };

    struct PropertyPathAtom {
        bool inverse;
        boost::variant<PropertyPathSuffix, PropertyPathBoundSuffix> suffix;

        boost::variant<
            std::string,
            boost::recursive_wrapper<PropertyPathAlternatives>
        > atom;

        PropertyPathAtom() :
            inverse(false) { }
    };

    struct PropertyPath {
        EdgeDirection direction;
        PropertyPathAlternatives path_alternatives;
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

    struct SelectStatement {
        bool distinct;
        std::vector<SelectItem> selection;
    };

    struct Root {
        bool                                             explain;
        SelectStatement                                  select;
        GraphPattern                                     graph_pattern;
        boost::optional<Formula>                         where;
        boost::optional<std::vector<OrderedSelectItem>>  group_by;
        boost::optional<std::vector<OrderedSelectItem>>  order_by;
        boost::optional<uint_fast32_t>                   limit;
    };
}}

#endif // BASE__QUERY_AST_H_
