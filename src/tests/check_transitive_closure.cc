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
#include "relational_model/execution/binding_id_iter/transitive_closure.h"
#include "relational_model/execution/binding_id_iter/transitive_closure_enum.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;
namespace po = boost::program_options;

// Antes de probar el test se espera que se cargó una base de datos en la carpeta correcta, ej:
// build/Release/bin/create_db tests/dbs/db_name.txt tests/dbs/db_name
int main(int argc, char **argv) {
    int buffer_size;
    string db_folder;

    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
            ("buffer-size,b", po::value<int>(&buffer_size)->default_value(BufferManager::DEFAULT_BUFFER_POOL_SIZE),
                "set buffer pool size")
        ;

        po::positional_options_description p;
        p.add("db-folder", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: server [options] DB_FOLDER\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        QuadModel model(db_folder, buffer_size);

        auto& bpt = *model.type_from_to_edge; //from: 1, to: 2, type: 0
        auto start_id = ObjectId(model.get_identifiable_object_id("Q0"));
        // auto end_id   = ObjectId(model.get_identifiable_object_id("Q94846"));
        auto type_id  = ObjectId(model.get_identifiable_object_id("knows"));
        cout << "before creating TransitiveClosure\n";
        // auto op = TransitiveClosure(4, bpt, start_id, end_id, type_id, 0, 1, 2); // Boolean case

        auto op = TransitiveClosureEnum(4, bpt, start_id, VarId(1), type_id, 1, 0); // Enum case
        cout << "after creating TransitiveClosure\n";

        // Transitive Closure Test
        BindingId binding(4);
        auto& result = op.begin(binding);
        while (op.next()) {
            auto obj_id = result[VarId(1)];
            auto obj = model.get_graph_object(obj_id);
            cout << "Resultado: " << obj->to_string() << "\n";
        }
        cout << "Fin\n";
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
