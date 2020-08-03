#ifndef RELATIONAL_MODEL__MANUAL_PLAN_GRAMMAR_H_
#define RELATIONAL_MODEL__MANUAL_PLAN_GRAMMAR_H_

#include "relational_model/manual_plan/grammar/manual_plan_ast.h"
#include "relational_model/manual_plan/grammar/manual_plan_ast_adapted.h"

#include <boost/spirit/home/x3.hpp>

namespace manual_plan_parser
{
    namespace x3 = boost::spirit::x3;

    using x3::lit;
    using x3::lexeme;
    using x3::no_case;
    using x3::alnum;
    using x3::graph;
    using x3::int64;
    using x3::int32;
    x3::real_parser<float, x3::strict_real_policies<float> > const float_ = {};
    using x3::attr;
    using x3::omit;
    using x3::space;
    using x3::skip;
    using x3::no_skip;
    using x3::eol;
    using x3::eoi;

    using x3::char_;

    auto const line_skipper = "//" >> *(char_ - eol) >> (eol | eoi);

    // Declare skipper
    auto const skipper = space | line_skipper;

    // Declare rules
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
    x3::rule<class value, manual_plan_ast::Value>
        value = "value";


    ///////////////////////////////////////////////////////////
    //   GRAMMAR
    ///////////////////////////////////////////////////////////
    auto const var =
        lexeme['?' >> +(alnum | '_')];

    auto const key =
        lexeme[+char_("A-Za-zÁÉÍÓÚáéíóúÑñèç0-9#'_")];

    auto const label =
        lexeme[':' >> +char_("A-Za-zÁÉÍÓÚáéíóúÑñèç0-9#'_")];

    auto const boolean =
        (lexeme[no_case["true"]] >> attr(true)) | lexeme[no_case["false"]] >> attr(false);

    auto const string =
        (lexeme['"' >> *(char_ - '"') >> '"']) |
        (lexeme['\'' >> *(char_ - '\'') >> '\'']);

    auto const value_def =
        string | float_ | int64 | boolean;

    auto const node_label_def =
        lexeme["NodeLabel"] >> "(" >> var >> "," >> string >> ")";

    auto const edge_label_def =
        lexeme["EdgeLabel"] >> "(" >> var >> "," >> string >> ")";

    auto const node_property_def =
        lexeme["NodeProperty"] >> "(" >> var >> "," >> string >> "," >> value >> ")";

    auto const edge_property_def =
        lexeme["EdgeProperty"] >> "(" >> var >> "," >> string >> "," >> value >> ")";

    auto const connection_def =
        lexeme["Connection"] >> "(" >> var >> "," >> var >> "," >> var >> ")";

    auto const labeled_connection_def =
        lexeme["LabeledConnection"] >> "(" >> string >> "," >> var >> "," >> var >> "," >> var >> ")";

    auto const root_def =
        (node_label | node_property | edge_label | edge_property | connection | labeled_connection) % (',');

    BOOST_SPIRIT_DEFINE(
        root,
        node_label,
        edge_label,
        node_property,
        edge_property,
        connection,
        labeled_connection,
        value
    );
}

#endif // RELATIONAL_MODEL__MANUAL_PLAN_GRAMMAR_H_
