#include <chrono>
#include <climits>
#include <iostream>

#include "base/binding/binding_id.h"
#include "base/ids/var_id.h"
#include "execution/binding_id_iter/index_nested_loop_join.h"
#include "execution/binding_id_iter/index_scan.h"
#include "import/quad_model/bulk_import.h"
#include "query_optimizer/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/filesystem.h"
#include "storage/file_manager.h"
#include "third_party/cxxopts/cxxopts.h"

using namespace std;

int main(int argc, char **argv) {
    string input_filename;
    string db_folder;
    int buffer_size;

	try {
        cxxopts::Options options("create_db", "Import a database from a text file");
        options.add_options()
            ("h,help", "Print usage")
            ("d,db-folder", "path to the database folder to be created", cxxopts::value<string>(db_folder))
            ("b,buffer-size", "set buffer pool size", cxxopts::value<int>(buffer_size)->default_value(
                std::to_string(BufferManager::DEFAULT_SHARED_BUFFER_POOL_SIZE)))
            ("f,file", "file path to be imported", cxxopts::value<string>(input_filename))
        ;

        options.positional_help("import-file db-folder");
        options.parse_positional({"file", "db-folder"});

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (input_filename.empty()) {
            cerr << "Must specify an import file.\n";
            return 1;
        }

        if (db_folder.empty()) {
            cerr << "Must specify a db-folder.\n";
            return 1;
        }

        // Validate params
        if (buffer_size < 0) {
            cerr << "Buffer size cannot be a negative number.\n";
            return 1;
        }

        { // check db_folder is empty or does not exists
            if (Filesystem::exists(db_folder) && !Filesystem::is_empty(db_folder)) {
                cout << "Database folder already exists and it's not empty\n";
                return EXIT_FAILURE;
            }
        }

        cout << "Creating new database\n";
        cout << "  input file:  " << input_filename << "\n";
        cout << "  db folder:   " << db_folder << "\n";
        cout << "  buffer size: " << buffer_size << "\n\n";

        auto start = chrono::system_clock::now();
        cout << "Initializing system...\n";
        {
            // will initialize the model. When it comes out of scope the model is destroyed.
            auto model_destroyer = QuadModel::init(db_folder, buffer_size, 0, 0);

            // to measure time initializing the model
            auto end_model = chrono::system_clock::now();
            chrono::duration<float, milli> model_duration = end_model - start;
            cout << "  done in " << model_duration.count() << " ms\n\n";

            // start the import
            auto import = BulkImport(input_filename);
            import.start_import();

        }
        auto end = chrono::system_clock::now();
        chrono::duration<float, milli> duration = end - start;
        cout << "Total duration: " << duration.count() << " ms\n";

        return EXIT_SUCCESS;
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
        return EXIT_FAILURE;
    }
}
