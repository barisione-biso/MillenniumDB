#ifndef BASE__COMMON_AST_ADAPTED_H_
#define BASE__COMMON_AST_ADAPTED_H_

#include <boost/fusion/include/adapt_struct.hpp>

#include "base/parser/grammar/common/common_ast.h"

BOOST_FUSION_ADAPT_STRUCT(ast::Var,
    name
)

BOOST_FUSION_ADAPT_STRUCT(ast::Property,
    key, value
)

#endif // BASE__COMMON_AST_ADAPTED_H_
