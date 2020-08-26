#ifndef BASE__IMPORT_AST_ADAPTED_H_
#define BASE__IMPORT_AST_ADAPTED_H_

#include "import_ast.h"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(import_ast::Node,
    anonymous, name, labels, properties
)

BOOST_FUSION_ADAPT_STRUCT(import_ast::Edge,
    left_anonymous, left_name, direction, right_anonymous, right_name, labels, properties
)

#endif // BASE__IMPORT_AST_ADAPTED_H_
