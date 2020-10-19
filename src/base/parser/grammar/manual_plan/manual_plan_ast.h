#ifndef BASE__MANUAL_PLAN_AST_H_
#define BASE__MANUAL_PLAN_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/parser/grammar/common/common_ast.h"

namespace manual_plan { namespace ast {
    namespace x3 = boost::spirit::x3;
    using namespace common::ast;

    struct NodeLabel {
        std::string var_or_id;
        std::string label;
    };

    struct ObjectProperty {
        std::string var_or_id;
        std::string key;
        ast::Value value;
    };

    // struct Connection {
    //     std::string from;
    //     std::string to;
    //     std::string edge;
    // };

    // struct ConnectionType {
    //     std::string type;
    //     std::string edge;
    // };

    struct TypedConnection {
        std::string from;
        std::string to;
        std::string type;
        std::string edge;
    };

    using Relation = boost::variant<NodeLabel, ObjectProperty, TypedConnection>;

    struct Root {
        std::vector<Relation> relations;
    };
}}

#endif // BASE__MANUAL_PLAN_AST_H_
