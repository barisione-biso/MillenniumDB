#include <iostream>
#include <map>
#include <string>

#include "base/ids/object_id.h"
#include "query_optimizer/rdf_model/rdf_model.h" // TODO moverlo
#include "network/service/consume_api.h"
#include "network/service/response_parser.h"

int main(int argc, char** argv){
    try {
        if(argc != 6){
            std::cerr <<
                "Usage: build/Release/bin/service_ssl <host> <port> <target> <format> <https?>\n" <<
                "Example:\n" <<
                "    build/Release/bin/service_ssl www.example.com 80 xml true\n" <<
                "    build/Release/bin/service_ssl query.wikidata.org 443 /sparql?query={SPARQL} json true\n";
            return EXIT_FAILURE;
        }
        std::string const host = argv[1];
        std::string const port = argv[2];
        std::string const target = argv[3];
        std::string const raw_format = argv[4];
        bool const https = std::strcmp(argv[5], "false") ? true : false;

        if (raw_format != "json" && raw_format != "xml" && raw_format != "csv" && raw_format != "tsv"){
            std::cerr << "Wrong format. Format argument must be one of: json, xml, csv, tsv.";
            return EXIT_FAILURE;
        }

        Format format = Format::json;
        if(raw_format == "xml") format = Format::xml;
        else if(raw_format == "csv") format = Format::csv;
        else if(raw_format == "tsv") format = Format::tsv;

        auto model_destroyer = RdfModel::init("tests/dbs/sparql_db", 1024, 1024, 1);

        std::string body_response = consume(host, port, target, raw_format, https);
        // std::cout << "______________" << std::endl;
        // std::cout << body_response << std::endl;
        // std::cout << "______________" << std::endl;
        // return EXIT_SUCCESS;
        Parser parser = Parser(format);
        parser.begin(std::move(body_response));
        std::map<std::string, ObjectId> binding;

        std::string attrs[3] = {"s", "p", "o"};


        while (parser.next(binding)){
            bool first_var = true;
            for (auto& attr: attrs){
                if(!first_var) std::cout << ",";
                first_var = false;
                auto item = binding.find(attr);
                if (item != binding.end()) {
                    std::cout << rdf_model.get_graph_object(item->second);
                }
            }
            std::cout << std::endl;
        }

    } catch(std::exception const& e){
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}