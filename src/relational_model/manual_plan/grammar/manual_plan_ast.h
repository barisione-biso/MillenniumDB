#ifndef RELATIONAL_MODEL__MANUAL_PLAN_AST_H_
#define RELATIONAL_MODEL__MANUAL_PLAN_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"

namespace manual_plan_ast
{
    namespace x3 = boost::spirit::x3;

    using Value = boost::variant<std::string, int64_t, float, bool>;

    struct NodeLabel {
        std::string node_name;
        std::string label;
    };

    struct EdgeLabel {
        std::string edge_name;
        std::string label;
    };

    struct NodeProperty {
        std::string node_name;
        std::string key;
        Value value;
    };

    struct EdgeProperty {
        std::string edge_name;
        std::string key;
        Value value;
    };

    struct Connection {
        std::string edge_name;
        std::string node_from_name;
        std::string node_to_name;
    };

    struct LabeledConnection {
        std::string label;
        std::string edge_name;
        std::string node_from_name;
        std::string node_to_name;
    };

    using Relation = boost::variant<NodeLabel, EdgeLabel, NodeProperty, EdgeProperty, Connection, LabeledConnection>;

    struct Root {
        std::vector<Relation> relations;
    };
}

#endif // RELATIONAL_MODEL__MANUAL_PLAN_AST_H_
