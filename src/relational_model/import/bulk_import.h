#ifndef RELATIONAL_MODEL__BULK_IMPORT_H_
#define RELATIONAL_MODEL__BULK_IMPORT_H_

#include "storage/index/ordered_file/ordered_file.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/import/bulk_import_ast.h"

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <memory>

class BulkImport {
public:
    BulkImport(const std::string& nodes_file, const std::string& edges_file, RelationalGraph& graph);
    ~BulkImport() = default;

    void start_import();

private:
    RelationalGraph& graph;

    std::ifstream nodes_file;
    std::ifstream edges_file;

    OrderedFile<2> node_labels;             // node_id, label_id
    OrderedFile<2> edge_labels;             // edge_id, label_id
    OrderedFile<3> node_key_value;          // node_id, key_id, value_id
    OrderedFile<3> edge_key_value;          // edge_id, key_id, value_id
    OrderedFile<3> connections;             // from_id, to_id, edge_id
    OrderedFile<2> self_connected_nodes;    // node_id, edge_id
    OrderedFile<4> labeled_edges;           // label_id, from_id, to_id, edge_id

    std::map<uint64_t, uint64_t> node_dict;

    void process_node(const bulk_import_ast::Node& node);
    void process_edge(const bulk_import_ast::Edge& edge);

    template <std::size_t N>
    void merge_tree_and_ordered_file(std::unique_ptr<BPlusTree<N>>&, OrderedFile<N>&);
};

#endif // RELATIONAL_MODEL__BULK_IMPORT_H_