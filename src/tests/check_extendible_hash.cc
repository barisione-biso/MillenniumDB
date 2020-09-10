/**
 * Read all strings of a database and checks its present on the ExtendibleHash and can be found
 */

#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    string db_folder;

    // Parse arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "show this help message")
        ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
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

    // TODO: remake
    // RelationalModel::init(db_folder, BufferManager::DEFAULT_BUFFER_POOL_SIZE);

    // // TODO: storage sould not include relational_model
    // auto& object_file = relational_model.get_object_file();
    // auto& strings_hash = relational_model.get_strings_hash();

    // uint64_t current_id = 1;

    // uint64_t correct = 0;
    // uint64_t wrong = 0;
    // try {
    //     while (true) {
    //         auto bytes = object_file.read(current_id);
    //         string str(bytes->begin(), bytes->end());

    //         auto id_found = strings_hash.get_id(str);
    //         if (current_id != id_found) {
    //             ++wrong;
    //             int32_t diff = current_id - id_found;
    //             cerr << "Wrong ID for string \"" << str << "\", reald id: " << current_id
    //                 << ", found: " << id_found << ", diff: " << diff << "\n";
    //         } else {
    //             ++correct;
    //         }

    //         current_id += bytes->size() + 4;
    //     }
    // }
    // catch (exception& e) {
    //     cerr << "Exception: " << e.what() << "\n";
    // }
    // cout << "Wrong:   " << wrong << "\n";
    // cout << "Correct: " << correct << "\n";
}
