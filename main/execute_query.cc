#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>

#include "grammar/ast.h"
#include "grammar/ast_adapted.h"
#include "grammar/grammar.h"
#include "grammar/visitors.h"


#include "base/graph/value/value_string.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/query_optimizer.h"
#include "relational_model/query_optimizer/query_optimizer_label.h"
#include "relational_model/query_optimizer/query_optimizer_property.h"
#include "relational_model/query_optimizer/query_optimizer_connection.h"


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
        cout << "Parsing succeeded\n";
        try
        {
            visitors::firstVisitor visit1;

            map<string, unsigned> id_map = visit1(ast); // variable name -> variable_id

            visitors::secondVisitor visit2(id_map);
            visitors::thirdVisitor  visit3(id_map);
            visitors::fourthVisitor visit4(id_map);
            visitors::fifthVisitor  visit5(id_map);

            map<unsigned, unsigned> entities_map = visit2(ast);
            map<unsigned, vector<string>> labels_map = visit3(ast);
            map<unsigned, map<string, ast::value>> properties_map = visit4(ast);
            vector<array<unsigned, 3>> connections = visit5(ast);

            Config config = Config();
            RelationalGraph graph = RelationalGraph(0, config);

            std::vector<QueryOptimizerElement*> elements {};
            VarId null_var {-1 };

            // ADD LABELS
            for (auto&& [element_id, labels] : labels_map) {
                ElementType element_type = ElementType::NODE; // TODO: check
                VarId element_obj_id = VarId(element_id);
                for (auto& label : labels) {
                    VarId label_var = null_var; // TODO: no considera labels con variable
                    ObjectId label_id = graph.get_label_id(Label(label)); // TODO: no considera labels con variable

                    elements.push_back(new QueryOptimizerLabel(
                        graph, element_obj_id, label_var, element_type, label_id
                    ));
                }
            }

            // ADD PROPERTIES
            for (auto&& [element_id, key_value] : properties_map) {
                ElementType element_type = ElementType::NODE; // TODO: check
                VarId element_obj_id = VarId(element_id);

                for (auto&& [key, value] : key_value) {
                    VarId key_var = null_var;   // TODO: no considera con variable
                    VarId value_var = null_var; // TODO: no considera con variable
                    ObjectId key_id = graph.get_key_id(Key(key)); // TODO: no considera con variable
                    ObjectId value_id;
                    int value_type = value.which();
                    if (value_type == 0) {
                        auto val_str = boost::get<string>(value);
                        value_id = graph.get_value_id(ValueString(val_str));
                    }
                    else {
                        cerr << "Error: only string values supported for now. got value_type " << value_type << ".\n" ;
                    }
                    elements.push_back(new QueryOptimizerProperty(
                        graph, element_obj_id, key_var, value_var, element_type, key_id, value_id
                    ));
                }
            }
            QueryOptimizer optimizer{};
            auto root = optimizer.get_query_plan(elements);

            // std::vector<std::string> var_names {
            //     "?n",
            //     "?n.name"
            // };
            vector<string> var_names(id_map.size());
            for (auto&&[var_name, var_id] : id_map) {
                cout << "var_names["<<var_id<<"] = " << var_name << endl;
                var_names[var_id] = var_name;
            }
            auto input = make_shared<BindingId>(var_names.size());

            root->init(input);
            unique_ptr<BindingId const> b = root->next();
            int count = 0;
            while (b != nullptr) {
                b->print(var_names);
                b = root->next();
                count++;
            }
            cout << "Found " << count << " results.\n";


        } catch (const std::exception& e) {
            cerr << "Error while processing query:\n" << e.what() << endl;
            return 1;
        }
        return 0;
    }
    else
    {
        cerr << "Parsing failed\n";
        return 1;
    }
}