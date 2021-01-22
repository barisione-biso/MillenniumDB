#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>

#include <boost/program_options.hpp>

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/graph/graph_model.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/query_parser.h"
#include "relational_model/execution/binding_id_iter/transitive_closure_check.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    int buffer_size;
    string db_folder;
    string start_node;
    string end_node;
    string connection_type;

    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("buffer-size,b", po::value<int>(&buffer_size)->default_value(BufferManager::DEFAULT_BUFFER_POOL_SIZE),
                "set buffer pool size")
            ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
            ("start,s", po::value<string>(&start_node)->required(), "set start node")
            ("end,e", po::value<string>(&end_node)->required(), "set end node")
            ("type,t", po::value<string>(&connection_type)->required(), "set connection type")
        ;

        po::positional_options_description p;
        p.add("db-folder", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: build/Release/bin/check_transitive_closure_enum -d [DATABASE_FOLDER] -s [START_NODE] -e [END_NODE] -t [TYPE]\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        QuadModel model(db_folder, buffer_size);

        auto& bpt = *model.type_from_to_edge; // from: 1, to: 2, type: 0
        auto start_id = ObjectId(model.get_identifiable_object_id(start_node));
        auto end_id   = ObjectId(model.get_identifiable_object_id(end_node));
        auto type_id  = ObjectId(model.get_identifiable_object_id(connection_type));
        auto op = TransitiveClosureCheck(4, bpt, start_id, end_id, type_id, 1, 0);

        // Transitive Closure Test
        BindingId binding(4);
        op.begin(binding);
        if (op.next()) {
            cout << "Path found\n";
        } else {
            cout << "Path not found\n";
        }
        if (op.next()) {
            cerr << "Error: TransitiveClosureCheck shouldn't give more than 1 result unless reset() is called.\n";
            return 1;
        }
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
        return 1;
    }
    return 0;
}
