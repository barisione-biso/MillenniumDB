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
            visitors::assignVarIDs visit1;
            map<string, uint_fast32_t> id_map = visit1.visit(ast); // variable name -> variable_id

            visitors::assignEntities    visit2(id_map);
            visitors::assignLabels      visit3(id_map);
            visitors::asssignProperties visit4(id_map);
            visitors::assignConnections visit5(id_map);

            visit2(ast);
            auto id2type = visit2.get_id2type();
            visit3(ast);
            map<uint_fast32_t, vector<string>> labels_map = visit3.get_labelMap();
            visit4(ast);
            map<uint_fast32_t, map<string, ast::value>> properties_map = visit4.get_property_map();
            visit5(ast);
            vector<array<uint_fast32_t, 3>> connections = visit5.get_connections();

            Config config = Config();
            RelationalGraph graph = RelationalGraph(0, config);

            std::vector<QueryOptimizerElement*> elements {};
            VarId null_var { -1 };

            // ADD LABELS
            for (auto&& [element_id, labels] : labels_map) {
                ElementType element_type = id2type[element_id];
                VarId element_obj_id = VarId(element_id);
                for (auto& label : labels) {
                    VarId label_var = null_var;
                    ObjectId label_id = graph.get_label_id(Label(label));

                    elements.push_back(new QueryOptimizerLabel(
                        graph, element_obj_id, label_var, element_type, label_id
                    ));
                }
            }

            // ADD PROPERTIES
            for (auto&& [element_id, key_value] : properties_map) {
                ElementType element_type = id2type[element_id];
                VarId element_obj_id = VarId(element_id);

                for (auto&& [key, value] : key_value) {
                    VarId key_var = null_var;
                    VarId value_var = null_var;
                    ObjectId key_id = graph.get_key_id(Key(key));
                    ObjectId value_id;

                    visitors::getValue visit_value;
                    auto val_ptr = visit_value(value);
                    if (val_ptr->is_var()) {
                        value_var = val_ptr->get_var();
                    }
                    else {
                        value_id = graph.get_value_id(*val_ptr);
                    }
                    elements.push_back(new QueryOptimizerProperty(
                        graph, element_obj_id, key_var, value_var, element_type, key_id, value_id
                    ));
                }
            }
            // ADD CONNECTIONS
            for (auto const& connection: connections) {
                elements.push_back(new QueryOptimizerConnection(
                    graph, VarId(connection[0]), VarId(connection[2]), VarId(connection[1])
                ));
            }

            QueryOptimizer optimizer{};
            auto& root = optimizer.get_query_plan(elements);

            vector<string> var_names(id_map.size());
            for (auto&&[var_name, var_id] : id_map) {
                cout << "var_names["<<var_id<<"] = " << var_name << endl;
                var_names[var_id] = var_name;
            }
            auto input = BindingId(var_names.size());

            root.init(input);
            BindingId* b = root.next();
            int count = 0;
            while (b != nullptr) {
                b->print(var_names, config.get_object_file());
                b = root.next();
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
