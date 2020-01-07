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
    bool r = phrase_parse(iter, end, parser::root, parser::skipper, ast);

    if (r && iter == end)
    {
        cout << "-------------------------\n";
        cout << "Parsing succeeded\n";
        cout << "-------------------------\n";
        // Get AST printer
        visitors::printer printer(cout);
        
        // Get first visitor
        visitors::firstVisitor visit1;

        try 
        {
            // printer(ast);
            map<string, unsigned> idMap = visit1(ast);
            // printer(ast);

            // Print map obtained
            cout << "\nMap obtained at first step:\n";
            for(auto const& t: idMap) {
                cout << "Variable(" << t.first << ") -> " << "VarId(" << t.second << "),\n";
            }

            // Get following visitors
            visitors::secondVisitor visit2(idMap);
            visitors::thirdVisitor visit3(idMap);
            visitors::fourthVisitor visit4(idMap);
            visitors::fifthVisitor visit5(idMap);

            map<unsigned, unsigned> entMap = visit2(ast);

            cout << "\nMap obtained at second step:\n";
            string s;
            for(auto const& t: entMap) {
                switch (t.second)
                {
                case NODE:
                    s = "NODE";
                    break;
                case EDGE:
                    s = "EDGE";
                    break;
                default:
                    break;
                }
                cout << "Entity(" << t.first << ", " << s << "),\n"; 
            }

            // 3rd Visitor
            map<unsigned, vector<string>> labelMap = visit3(ast);

            // Print map obtained
            cout << "\nMap obtained at third step:\n";
            for(auto const& t: labelMap) {
                for(auto const& d: t.second) {
                    cout << "Label(" << t.first << ", " <<  d << "),\n";
                }
                
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
        } catch (const std::exception& e) {
            cerr << "Error while processing query:\n" << e.what() << endl;
            return 1;
        }
        

        return 0;
    }
    else
    {
        string::const_iterator some = iter+30;
        string context(iter, (some>end)?end:some);
        cout << "-------------------------\n";
        cerr << "Parsing failed\n";
        cerr << "stopped at: \": " << context << "...\"\n";
        cout << "-------------------------\n";
        return 1;
    }
}