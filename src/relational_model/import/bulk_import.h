#ifndef RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H
#define RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <regex>

#include "file/index/ordered_file/ordered_file.h"

class RelationalGraph;

class BulkImport {
public:
    BulkImport(const std::string& nodes_file, const std::string& edges_file, RelationalGraph& graph);
    ~BulkImport() = default;

    void start_import();

private:
    std::ifstream nodes_file;
    std::ifstream edges_file;
    RelationalGraph& graph;

    void process_node(const std::string& line, int line_number);
    void process_edge(const std::string& line, int line_number);

    uint64_t node_count;
    uint64_t edge_count;
    std::map<uint64_t, uint64_t> node_dict;
    std::list<std::tuple<uint64_t, uint64_t, uint64_t>> edges_original_ids;

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