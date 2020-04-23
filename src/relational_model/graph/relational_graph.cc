#include "relational_model/graph/relational_graph.h"

#include "base/graph/edge.h"
#include "base/graph/node.h"
#include "base/graph/value/value_string.h"
#include "storage/buffer_manager.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/relational_model.h"

#include <openssl/md5.h>

using namespace std;

RelationalGraph::RelationalGraph(GraphId graph_id)
    : graph_id(graph_id) { }


RelationalGraph::~RelationalGraph() = default;


uint64_t RelationalGraph::create_node() {
    return relational_model.get_catalog().create_node(graph_id)
           | RelationalModel::NODE_MASK
           | (graph_id << RelationalModel::GRAPH_OFFSET);
}


uint64_t RelationalGraph::create_edge() {
    return relational_model.get_catalog().create_edge(graph_id)
           | RelationalModel::EDGE_MASK
           | (graph_id << RelationalModel::GRAPH_OFFSET);
}


Record RelationalGraph::get_record_for_node_label(uint64_t node_id, const string& label) {
    uint64_t label_id = relational_model.get_or_create_string_unmasked_id(label);
    relational_model.get_catalog().add_node_label(graph_id, label_id);
    return Record(
        node_id,
        label_id | (graph_id << RelationalModel::GRAPH_OFFSET)
    );
}


Record RelationalGraph::get_record_for_edge_label(uint64_t edge_id, const string& label) {
    uint64_t label_id = relational_model.get_or_create_string_unmasked_id(label);
    relational_model.get_catalog().add_edge_label(graph_id, label_id);
    return Record(
        edge_id,
        label_id | (graph_id << RelationalModel::GRAPH_OFFSET)
    );
}


Record RelationalGraph::get_record_for_node_property(uint64_t node_id, const string& key, const Value& value) {
    uint64_t key_id = relational_model.get_or_create_string_unmasked_id(key);
    uint64_t value_id = relational_model.get_or_create_value_masked_id(value);

    relational_model.get_catalog().add_node_key(graph_id, key_id);
    return Record(
        node_id,
        key_id | (graph_id << RelationalModel::GRAPH_OFFSET),
        value_id | (graph_id << RelationalModel::GRAPH_OFFSET)
    );
}


Record RelationalGraph::get_record_for_edge_property(uint64_t edge_id, const string& key, const Value& value) {
    uint64_t key_id = relational_model.get_or_create_string_unmasked_id(key);
    uint64_t value_id = relational_model.get_or_create_value_masked_id(value);

    relational_model.get_catalog().add_edge_key(graph_id, key_id);
    return Record(
        edge_id,
        key_id | (graph_id << RelationalModel::GRAPH_OFFSET),
        value_id | (graph_id << RelationalModel::GRAPH_OFFSET)
    );
}
