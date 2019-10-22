#ifndef RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H
#define RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H

#include <string>
#include <fstream>
#include <list>
#include <map>

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

    void process_node(const string& line);
    void process_edge(const string& line);

    uint64_t node_count;
    uint64_t edge_count;
    map<uint64_t, uint64_t> node_dict;
    list<tuple<uint64_t, uint64_t, uint64_t>> edges_original_ids;
};

#endif //RELATIONAL_MODEL__IMPORT__BULK_IMPORT_H