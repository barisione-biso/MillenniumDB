#ifndef BASE__MANUAL_PLAN_AST_H_
#define BASE__MANUAL_PLAN_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"
#include "base/parser/grammar/common/common_ast.h"

namespace manual_plan_ast {
    namespace x3 = boost::spirit::x3;
    using namespace ast;

    struct NodeLabel {
        Var node;
        std::string label;
    };

    struct EdgeLabel {
        Var edge;
        std::string label;
    };

    struct NodeProperty {
        Var node;
        std::string key;
        ast::Value value;
    };

    struct EdgeProperty {
        Var edge;
        std::string key;
        ast::Value value;
    };

    struct Connection {
        Var edge;
        Var node_from;
        Var node_to;
    };

    struct LabeledConnection {
        std::string label;
        Var edge;
        Var node_from;
        Var node_to;
    };

    using Relation = boost::variant<NodeLabel, EdgeLabel, NodeProperty, EdgeProperty, Connection, LabeledConnection>;

    struct Root {
        std::vector<Relation> relations;
    };
}

#endif // BASE__MANUAL_PLAN_AST_H_
