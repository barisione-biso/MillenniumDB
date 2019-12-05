#if !defined(GRAMMAR_H)
#define GRAMMAR_H

#include <boost/spirit/home/x3.hpp>

#include "ast.h"

namespace parser
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = boost::spirit::x3::ascii;

    using x3::lit;
    using x3::lexeme;
    using x3::no_case;
    using x3::alnum;
    using x3::int_;
    using x3::double_;
    using x3::attr;

    using ascii::char_;

    // Declare rules
    x3::rule<class root, ast::root> 
        root = "root";
    x3::rule<class element, ast::element>
        element = "element";
    x3::rule<class linear_pattern, ast::linear_pattern> 
        linear_pattern = "linear_pattern";
    x3::rule<class node, ast::node> 
        node = "node";
    x3::rule<class edge, ast::edge> 
        edge = "edge";
    x3::rule<class property, ast::property> 
        property = "property";
    

    ///////////////////////////////////////////////////////////
    //   GRAMMAR
    ///////////////////////////////////////////////////////////

    auto const var =
        '?' >> +(alnum); 
    
    auto const key =
        +(alnum); 

    auto const func =
        +(alnum);
    
    auto const label = 
        +(alnum);

    auto const boolean = 
        (no_case["true"] >> attr(true)) | no_case["false"] >> attr(false);

    auto const string = 
        (lexeme['"' >> *(char_ - '"') >> '"']) |
        (lexeme['\'' >> *(char_ - '\'') >> '\'']);
    
    auto const value =
        string | double_ | int_ | boolean;
    
    auto const property_def =
        key >> ':' >> value;
    
    auto const nomen = 
        -(var) >> *(':' >> label) >> -("{" >> (property % ',') >> "}");
        // -(var) >> *(':' >> label);

    auto const node_def =
        '(' >>  nomen >> ")";
        // '(' >> -key >> ':' >> (key % ',') >> ')';  
    
    auto const edge_def = 
        ("-[" >> nomen >> "]->" >> attr(true)) |
        ("<-[" >> nomen >> "]-" >> attr(false));
    
    auto const linear_pattern_def =
        node >> *(edge >> node);
        // edge >> edge;
    
    auto const selection =
        char_("*") | (element % ',');
    
    auto const match_statement =
        no_case["match"] >> ( linear_pattern % ',');
        // no_case["match"] >> key;

    auto const select_statement =
        no_case["select"] >> selection;

    auto const root_def = 
        select_statement >> match_statement;
        // select_statement; 

    auto const element_def =
        (attr("") >> var >> '.' >> key) |
        (func >> '(' >> var >> '.' >> key >> ')');

    BOOST_SPIRIT_DEFINE(
        root,
        element,
        linear_pattern,
        node,
        edge, 
        property
    );
}

#endif