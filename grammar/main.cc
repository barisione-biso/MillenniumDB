// #define BOOST_SPIRIT_X3_DEBUG

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>

#include "ast.h"
#include "ast_adapted.h"
#include "grammar.h"
#include "visitors.h"

using namespace std;

int main(int argc, char **argv)
{
    char const* filename;
    if (argc > 1) {
        filename = argv[1];
    }
    else {
        cerr << "Error: No input file provided." << endl;
        return 1;
    }

    ifstream in(filename, ios_base::in);

    if (!in) {
        cerr << "Error: Could not open input file: "
            << filename << endl;
        return 1;
    }

    string storage; // We will read the contents here.
    in.unsetf(ios::skipws); // No white space skipping!
    copy(
        istream_iterator<char>(in),
        istream_iterator<char>(),
        back_inserter(storage));

    ast::root ast; // Our tree

    using boost::spirit::x3::ascii::space;
    string::const_iterator iter = storage.begin();
    string::const_iterator end = storage.end();
    bool r = phrase_parse(iter, end, parser::root, space, ast);

    if (r && iter == end)
    {
        cout << "-------------------------\n";
        cout << "Parsing succeeded\n";
        cout << "-------------------------\n";
        // Get AST printer
        visitors::printer printer(cout);
        
        // Get visitors 
        visitors::firstVisitor visit1;
        visitors::secondVisitor visit2;

        printer(ast);
        visit1(ast);
        printer(ast);

        map<string, unsigned> idMap = visit2(ast);

        // Print map obtained
        cout << "\nMap obtained at second step:\n";
        for(auto const& t: idMap) {
            cout << "Variable(" << t.first << ") : " << "ObjectId(" << t.second << "),\n";
        }

        visitors::thirdVisitor visit3(idMap);
        visitors::fourthVisitor visit4(idMap);
        visitors::fifthVisitor visit5(idMap);

        // 3rd Visitor
        map<unsigned, string> labelMap = visit3(ast);

        // Print map obtained
        cout << "\nMap obtained at third step:\n";
        for(auto const& t: labelMap) {
            cout << "Label(" << t.first << ", " <<  t.second << "),\n";
        }

        // 4th Visitor
        map<unsigned, map<string, ast::value>> propertyMap = visit4(ast);

        cout << "\nMap obtained at fourth step:\n";
        
        for(auto const& t: propertyMap) {
            for(auto const &s: t.second) {
                cout << "Property(" << t.first << ", " << s.first << ", ";
                printer(s.second);
                cout << ")\n";
            }
        }

        // 5th Visitor
        vector<array<unsigned, 3>> connections = visit5(ast);

        // Print vector obtained
        cout << "\nVector obtained at fifth step:\n";
        for(auto const& t: connections) {
            cout << "Connection(" << t[0] << ", " << t[1] << ", " << t[2] << "),\n";
        }

        return 0;
    }
    else
    {
        string::const_iterator some = iter+30;
        string context(iter, (some>end)?end:some);
        cout << "-------------------------\n";
        cout << "Parsing failed\n";
        cout << "stopped at: \": " << context << "...\"\n";
        cout << "-------------------------\n";
        return 1;
    }
}