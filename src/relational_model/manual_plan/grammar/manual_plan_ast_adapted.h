#ifndef RELATIONAL_MODEL__MANUAL_PLAN_AST_ADAPTED_H_
#define RELATIONAL_MODEL__MANUAL_PLAN_AST_ADAPTED_H_

#include <boost/fusion/include/adapt_struct.hpp>

#include "relational_model/manual_plan/grammar/manual_plan_ast.h"

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::Root,
    relations
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::NodeLabel,
    node_name, label
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::EdgeLabel,
    edge_name, label
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::NodeProperty,
    node_name, key, value
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::EdgeProperty,
    edge_name, key, value
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::Connection,
    edge_name, node_from_name, node_to_name
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::LabeledConnection,
    label, edge_name, node_from_name, node_to_name
)

#endif // RELATIONAL_MODEL__MANUAL_PLAN_AST_ADAPTED_H_
