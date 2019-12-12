#if !defined(AST_ADAPTED_H)
#define AST_ADAPTED_H

#include <boost/fusion/include/adapt_struct.hpp>
#include "ast.h"

BOOST_FUSION_ADAPT_STRUCT(ast::root, 
    selection_, graphPattern_, where_
)

// BOOST_FUSION_ADAPT_STRUCT(ast::root, 
//     where_
// )

BOOST_FUSION_ADAPT_STRUCT(ast::linear_pattern,
    root_, path_
)

BOOST_FUSION_ADAPT_STRUCT(ast::step_path,
    edge_, node_
)

BOOST_FUSION_ADAPT_STRUCT(ast::node,
    variable_, labels_, properties_
)

BOOST_FUSION_ADAPT_STRUCT(ast::edge,
    variable_, labels_, properties_, isright_
)

BOOST_FUSION_ADAPT_STRUCT(ast::property,
    key_, value_
)

BOOST_FUSION_ADAPT_STRUCT(ast::element, 
    function, variable, key
)


BOOST_FUSION_ADAPT_STRUCT(ast::statement, 
    lhs_, comparator_, rhs_
)

BOOST_FUSION_ADAPT_STRUCT(ast::parenthesis, 
    formula_
)

BOOST_FUSION_ADAPT_STRUCT(ast::step_formula,
    op_, cond_
)

BOOST_FUSION_ADAPT_STRUCT(ast::formula,
    root_, path_
)

#endif
