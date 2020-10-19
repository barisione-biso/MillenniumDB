#ifndef BASE__QUERY_AST_ADAPTED_H_
#define BASE__QUERY_AST_ADAPTED_H_

#include <boost/fusion/include/adapt_struct.hpp>

#include "base/parser/grammar/query/query_ast.h"

BOOST_FUSION_ADAPT_STRUCT(query::ast::Root,
    explain, selection, graph_pattern, where, limit
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::LinearPattern,
    root, path
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::StepPath,
    edge, node
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::Node,
    var_or_id, labels, properties
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::Edge,
    var_or_id, types, properties, direction
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::SelectItem,
    var, key
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::Statement,
    lhs, comparator, rhs
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::StepFormula,
    op, condition
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::Formula,
    root, path
)

BOOST_FUSION_ADAPT_STRUCT(query::ast::Condition,
    negation, content
)

#endif // BASE__QUERY_AST_ADAPTED_H_
