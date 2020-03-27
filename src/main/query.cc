#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/program_options.hpp>

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/physical_plan_generator.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    string query_file;
    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>(), "set database folder path")
            ("buffer-size,b", po::value<int>(), "set buffer pool size")
            ("query-file,q", po::value<string>(&query_file)->required(), "query file")
        ;

        po::positional_options_description p;
        p.add("query-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: query [options] QUERY_FILE\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        if (vm.count("buffer-size")) {
            buffer_manager.buffer_pool_size = vm["buffer-size"].as<int>();
        }
        if (vm.count("db-folder")) {
            file_manager.db_folder = vm["db-folder"].as<string>();
        }

        // Read query-file
        ifstream in(query_file, ios_base::in);
        if (!in) {
            cerr << "Error: Could not open input file: " << query_file << endl;
            return 1;
        }

        stringstream str_stream;
        str_stream << in.rdbuf();
        string query = str_stream.str();

        // start timer
        auto start = chrono::system_clock::now();
        auto select_plan = Op::get_select_plan(query);

        buffer_manager.init();
        RelationalModel::init();

        PhysicalPlanGenerator plan_generator { };
        auto root = plan_generator.exec(*select_plan);

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
        cout << "Found " << count << " results.\n";
        cout << "Execution time: " << duration.count() << " milliseconds.\n";

    }
    catch(exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
}
