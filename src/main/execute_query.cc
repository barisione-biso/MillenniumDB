#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/query_optimizer.h"

#include <chrono>
#include <fstream>
#include <iostream>


using namespace std;


int main(int argc, char **argv) {
    auto start = chrono::system_clock::now();

    if (argc < 2) {
        cerr << "Error: No input file provided." << endl;
        return 1;
    }
    char const* filename = argv[1];
    ifstream in(filename, ios_base::in);
    if (!in) {
        cerr << "Error: Could not open input file: " << filename << endl;
        return 1;
    }

    stringstream str_stream;
    str_stream << in.rdbuf();
    string query = str_stream.str();

    auto select_plan = Op::get_select_plan(query);

    Config config = Config();
    RelationalGraph graph = RelationalGraph(0, config);
    QueryOptimizer optimizer { graph, config.get_object_file() };
    auto root = optimizer.get_select_plan(move(select_plan));

    root->begin();
    auto binding = root->next();
    int count = 0;
    while (binding != nullptr) {
        binding->print();
        binding = root->next();
        count++;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<float, std::milli> duration = end - start;
    cout << "Found " << count << " results in " << duration.count() << " milliseconds.\n";
}
