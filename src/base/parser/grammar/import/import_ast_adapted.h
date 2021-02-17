#ifndef BASE__IMPORT_AST_ADAPTED_H_
#define BASE__IMPORT_AST_ADAPTED_H_

#include "import_ast.h"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(import::ast::Node,
    name, labels, properties
)

BOOST_FUSION_ADAPT_STRUCT(import::ast::Edge,
    left_name, direction, right_name, labels, properties
)

BOOST_FUSION_ADAPT_STRUCT(import::ast::ImplicitEdge,
    nesting, direction, right_name, labels, properties
)

#endif // BASE__IMPORT_AST_ADAPTED_H_
