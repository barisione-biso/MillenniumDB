#ifndef BASE__MANUAL_PLAN_GRAMMAR_H_
#define BASE__MANUAL_PLAN_GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast_adapted.h"
#include "base/parser/grammar/common/common_grammar.h"

namespace manual_plan_parser {
    namespace x3 = boost::spirit::x3;
    using namespace common_parser;

    // Rules
    x3::rule<class root, manual_plan_ast::Root>
        root = "root";
    x3::rule<class node_label, manual_plan_ast::NodeLabel>
        node_label = "node_label";
    x3::rule<class edge_label, manual_plan_ast::EdgeLabel>
        edge_label = "edge_label";
    x3::rule<class node_property, manual_plan_ast::NodeProperty>
        node_property = "node_property";
    x3::rule<class edge_property, manual_plan_ast::EdgeProperty>
        edge_property = "edge_property";
    x3::rule<class connection, manual_plan_ast::Connection>
        connection = "connection";
    x3::rule<class labeled_connection, manual_plan_ast::LabeledConnection>
        labeled_connection = "labeled_connection";

    // Grammar
    auto const node_label_def =
        lexeme["NodeLabel"] >> "(" >> var >> "," >> common_parser::string >> ")";

    auto const edge_label_def =
        lexeme["EdgeLabel"] >> "(" >> var >> "," >> common_parser::string >> ")";

    auto const node_property_def =
        lexeme["NodeProperty"] >> "(" >> var >> "," >> common_parser::string >> "," >> value >> ")";

    auto const edge_property_def =
        lexeme["EdgeProperty"] >> "(" >> var >> "," >> common_parser::string >> "," >> value >> ")";

    auto const connection_def =
        lexeme["Connection"] >> "(" >> var >> "," >> var >> "," >> var >> ")";

    auto const labeled_connection_def =
        lexeme["LabeledConnection"] >> "(" >> common_parser::string >> "," >> var >> "," >> var >> "," >> var >> ")";

    auto const root_def =
        (node_label | node_property | edge_label | edge_property | connection | labeled_connection) % (',');

    BOOST_SPIRIT_DEFINE(
        root,
        node_label,
        edge_label,
        node_property,
        edge_property,
        connection,
        labeled_connection
    );
}

#endif // BASE__MANUAL_PLAN_GRAMMAR_H_
