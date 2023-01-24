#include <iostream>

#include "network/sparql/sparql_server.h"
#include "query_optimizer/rdf_model/rdf_model.h"
#include "storage/buffer_manager.h"
#include "storage/filesystem.h"
#include "third_party/cxxopts/cxxopts.h"

using namespace std;

int main(int argc, char* argv[]) {
    int seconds_timeout;
    int port;
    int shared_buffer_size;
    int private_buffer_size;
    int threads;
    string db_folder;

    ios_base::sync_with_stdio(false);

    try {
        // Parse arguments
        cxxopts::Options options("server", "MillenniumDB server");
        options.add_options()
            ("h,help", "Print usage")
            ("d,db-folder", "set database folder path", cxxopts::value<string>(db_folder))
            ("p,port", "database server port",
                cxxopts::value<int>(port)->default_value(std::to_string(SPARQL_SERVER::SparqlServer::DEFAULT_PORT)))
            ("t,timeout", "timeout (in seconds)", cxxopts::value<int>(seconds_timeout)->default_value("60"))
            ("b,buffer-size", "set shared buffer pool size",
                cxxopts::value<int>(shared_buffer_size)->default_value(std::to_string(BufferManager::DEFAULT_SHARED_BUFFER_POOL_SIZE)))
            ("private-buffer-size", "set private buffer pool size for each thread",
                cxxopts::value<int>(private_buffer_size)->default_value(std::to_string(BufferManager::DEFAULT_PRIVATE_BUFFER_POOL_SIZE)))
            ("threads", "set worker threads", cxxopts::value<int>(threads)->default_value("8")) // TODO: default depending on cpu?
        ;
        options.positional_help("db-folder");
        options.parse_positional({"db-folder"});

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            cout << options.help() << endl;
            exit(0);
        }

        // Validate params
        if (db_folder.empty()) {
            cerr << "Must specify a db-folder.\n";
            return 1;
        }

        if (port <= 0) {
            cerr << "Port must be a positive number.\n";
            return 1;
        }

        if (threads <= 0) {
            cerr << "Worker threads must be a positive number.\n";
            return 1;
        }

        if (seconds_timeout <= 0) {
            cerr << "Timeout must be a positive number.\n";
            return 1;
        }

        if (shared_buffer_size <= 0) {
            cerr << "Buffer size must be a positive number.\n";
            return 1;
        }

        if (private_buffer_size <= 0) {
            cerr << "Private buffer size must be a positive number.\n";
            return 1;
        }

        if (!Filesystem::exists(db_folder)) {
            cerr << "Database folder does not exists.\n";
            return 1;
        } else if (!Filesystem::is_directory(db_folder)) {
            cerr << "Database folder is not a directory.\n";
            return 1;
        }

        std::cout << "Initializing server..." << std::endl;
        auto model_destroyer = RdfModel::init(db_folder,
                                              shared_buffer_size,
                                              private_buffer_size,
                                              threads);

        rdf_model.catalog().print();

        SPARQL_SERVER::SparqlServer server;
        server.run(port, threads, std::chrono::seconds(seconds_timeout));

        return EXIT_SUCCESS;
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
        return 1;
    }
}
