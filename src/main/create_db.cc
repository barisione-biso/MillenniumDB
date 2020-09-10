#include "base/ids/var_id.h"

#include <chrono>
#include <climits>
#include <iostream>

#include <boost/program_options.hpp>

#include "relational_model/binding/binding_id.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/index_nested_loop_join.h"
#include "relational_model/models/quad_model/import/bulk_import.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    string filename;
    string db_folder;
    int buffer_size;

	try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
            ("buffer-size,b", po::value<int>(&buffer_size)->default_value(BufferManager::DEFAULT_BUFFER_POOL_SIZE),
                "set buffer pool size")
            ("filename,f", po::value<string>(&filename)->required(), "import file")
        ;

        po::positional_options_description p;
        p.add("filename",  1);
        p.add("db-folder", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: create_db ./path/to/import_file.txt ./path/to/new_db [OPTIONS]\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        cout << "filename: " << filename << "\n";
        cout << "db_folder: " << db_folder << "\n";
        cout << "buffer_size: " << buffer_size << "\n";

        auto model = QuadModel(db_folder, buffer_size);
        auto start = chrono::system_clock::now();

        auto import = BulkImport(filename, model);
        import.start_import();

        auto end = chrono::system_clock::now();
        chrono::duration<float, milli> duration = end - start;
        cout << "Bulk Import duration: " << duration.count() << "ms\n";
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}
