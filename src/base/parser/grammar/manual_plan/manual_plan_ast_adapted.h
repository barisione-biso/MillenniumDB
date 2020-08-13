#ifndef BASE__MANUAL_PLAN_AST_ADAPTED_H_
#define BASE__MANUAL_PLAN_AST_ADAPTED_H_

#include <boost/fusion/include/adapt_struct.hpp>

#include "base/parser/grammar/manual_plan/manual_plan_ast.h"

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::Root,
    relations
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::NodeLabel,
    node, label
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::EdgeLabel,
    edge, label
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::NodeProperty,
    node, key, value
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::EdgeProperty,
    edge, key, value
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::Connection,
    edge, node_from, node_to
)

BOOST_FUSION_ADAPT_STRUCT(manual_plan_ast::LabeledConnection,
    label, edge, node_from, node_to
)

#endif // BASE__MANUAL_PLAN_AST_ADAPTED_H_
