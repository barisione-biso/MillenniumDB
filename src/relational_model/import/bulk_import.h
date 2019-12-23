#ifndef RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H
#define RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <regex>

#include "file/index/ordered_file/ordered_file.h"

using namespace std;

class RelationalGraph;

class BulkImport {
public:
    BulkImport(const string& nodes_file, const string& edges_file, RelationalGraph& graph);
    ~BulkImport() = default;

    void start_import();

private:
    ifstream nodes_file;
    ifstream edges_file;
    RelationalGraph& graph;

    void process_node(const string& line, int line_number);
    void process_edge(const string& line, int line_number);

    uint64_t node_count;
    uint64_t edge_count;
    map<uint64_t, uint64_t> node_dict;
    list<tuple<uint64_t, uint64_t, uint64_t>> edges_original_ids;

    std::regex edge_line_expr;
    std::regex node_line_expr;
    std::regex node_expr;
    std::regex edge_expr;
    std::regex label_expr;
    std::regex properties_expr;

    OrderedFile node_labels;
    OrderedFile edge_labels;
    OrderedFile node_key_value;
    OrderedFile edge_key_value;
    OrderedFile from_to_edge;
};

#endif //RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H