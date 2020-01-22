#ifndef BASE__GRAMMAR_H_
#define BASE__GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "base/parser/grammar/ast.h"

namespace parser
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = boost::spirit::x3::ascii;

    using x3::lit;
    using x3::lexeme;
    using x3::no_case;
    using x3::alnum;
    using x3::graph;
    using x3::int_;
    using x3::float_;
    using x3::attr;
    using x3::omit;
    using x3::space;
    using x3::skip;
    using x3::no_skip;
    using x3::eol;
    using x3::eoi;
    // using x3::string;

    using ascii::char_;

    // Declare skipper
    auto const skipper =
        space | "//" >> *(char_ - eol) >> (eol | eoi);

    // Declare rules
    x3::rule<class root, ast::Root>
        root = "root";
    x3::rule<class element, ast::Element>
        element = "element";
    x3::rule<class linear_pattern, ast::LinearPattern>
        linear_pattern = "linear_pattern";
    x3::rule<class node, ast::Node>
        node = "node";
    x3::rule<class edge, ast::Edge>
        edge = "edge";
    x3::rule<class property, ast::Property>
        property = "property";
    x3::rule<class condition, ast::Condition>
        condition = "condition";
    x3::rule<class statement, ast::Statement>
        statement = "statement";
    x3::rule<class value, ast::Value>
        value = "value";
    x3::rule<class formula, ast::Formula>
        formula = "formula";


    ///////////////////////////////////////////////////////////
    //   GRAMMAR
    ///////////////////////////////////////////////////////////

    auto const comparator =
        lit("==") >> attr(ast::EQ()) |
        lit("<=") >> attr(ast::LE()) |
        lit(">=") >> attr(ast::GE()) |
        lit("!=") >> attr(ast::NE()) |
        lit('<')  >> attr(ast::LT()) |
        lit('>')  >> attr(ast::GT());

    auto const connector =
        (omit[+space] >> "AND" >> omit[+space] >> attr(ast::And())) |
        (omit[+space] >> "OR"  >> omit[+space] >> attr(ast::Or()));

    auto const var =
        '?' >> +(alnum);

    auto const key =
        +(char_("A-Za-z0-9#'&%$!|+-"));

    auto const func =
        +(alnum);

    auto const label =
        +(char_("A-Za-z0-9#'&%$!|+-"));

    auto const boolean =
        (no_case["true"] >> attr(true)) | no_case["false"] >> attr(false);

    auto const string =
        (lexeme['"' >> *(char_ - '"') >> '"']) |
        (lexeme['\'' >> *(char_ - '\'') >> '\'']);

    auto const value_def =
        string | float_ | int_ | boolean;

    auto const property_def =
        key >> ':' >> value;

    auto const nomen =
        -(var) >> *(':' >> label) >> -("{" >> -(property % ',') >> "}");

    auto const node_def =
        '(' >>  nomen >> ")";

    auto const edge_def =
        (-("-[" >> nomen >> ']') >> "->" >> attr(ast::EdgeDirection::right)) |
        ("<-" >> -('[' >> nomen >> "]-") >> attr(ast::EdgeDirection::left));

    auto const linear_pattern_def =
        node >> *(edge >> node);

    auto const selection =
        lit('*') >> attr(ast::All()) | (element % (',' >> omit[*space]));

    auto const statement_def =
        element >> omit[+space] >> comparator >> omit[+space] >> (element | value);

    auto const condition_def =
        -(lit("NOT") >> attr(true) >> omit[*space]) >>
        (
            statement |
            ('(' >> omit[*space] >> formula >> omit[*space] >> ')')
        );

    auto const formula_def =
        condition >> *(connector >> condition);

    auto const match_statement =
        no_case["match"] >> ( linear_pattern % ',');

    auto const select_statement =
        no_case["select"] >> omit[+space] >> selection;

    auto const where_statement =
        no_case["where"] >> omit[+space] >> formula;

    auto const root_def =
        no_skip[select_statement >> omit[+space] >> skip[match_statement]
        >> -(omit[+space] >> where_statement)];

    auto const element_def =
        (attr(std::string()) >> no_skip[var >> '.' >> key]) |
        (func >> '(' >> no_skip[var >> '.' >> key] >> ')');

    BOOST_SPIRIT_DEFINE(
        root,
        element,
        linear_pattern,
        node,
        edge,
        property,
        condition,
        statement,
        value,
        formula
    );
}

#endif // BASE__GRAMMAR_H_
