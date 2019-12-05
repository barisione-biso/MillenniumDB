#if !defined(AST_ADAPTED_H)
#define AST_ADAPTED_H

#include <boost/fusion/include/adapt_struct.hpp>
#include "ast.h"

BOOST_FUSION_ADAPT_STRUCT(ast::root, 
    selection_, graphPattern_
)

BOOST_FUSION_ADAPT_STRUCT(ast::linear_pattern,
    root_, path_)

BOOST_FUSION_ADAPT_STRUCT(ast::step_path,
    edge_, node_)

BOOST_FUSION_ADAPT_STRUCT(ast::node,
    variable_, labels_, properties_)

BOOST_FUSION_ADAPT_STRUCT(ast::edge,
    variable_, labels_, properties_, isright_)

BOOST_FUSION_ADAPT_STRUCT(ast::property,
    key_, value_)

BOOST_FUSION_ADAPT_STRUCT(ast::element, 
    function, variable, key
)


#endif
