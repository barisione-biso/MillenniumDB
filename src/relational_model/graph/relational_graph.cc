#include "relational_model/graph/relational_graph.h"

#include "base/graph/edge.h"
#include "base/graph/node.h"
#include "base/graph/value/value_string.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"
#include "storage/catalog/catalog.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"

using namespace std;

RelationalGraph::RelationalGraph(GraphId graph_id)
    : graph_id(graph_id) { }


RelationalGraph::~RelationalGraph() = default;


uint64_t RelationalGraph::create_node() {
    return catalog.create_node(graph_id)
           | RelationalModel::NODE_MASK
           | (graph_id << RelationalModel::GRAPH_OFFSET);
}


uint64_t RelationalGraph::create_edge(bool node_loop) {
    return catalog.create_edge(graph_id, node_loop)
           | RelationalModel::EDGE_MASK
           | (graph_id << RelationalModel::GRAPH_OFFSET);
}


Record<2> RelationalGraph::get_record_for_node_label(uint64_t node_id, const string& label) {
    auto label_id = relational_model.get_string_id(label, true).id;
    catalog.add_node_label(graph_id, label_id);
    return RecordFactory::get(
        node_id,
        label_id
    );
}


Record<2> RelationalGraph::get_record_for_edge_label(uint64_t edge_id, const string& label) {
    auto label_id = relational_model.get_string_id(label, true).id;
    catalog.add_edge_label(graph_id, label_id);
    return RecordFactory::get(
        edge_id,
        label_id
    );
}


Record<3> RelationalGraph::get_record_for_node_property(uint64_t node_id, const string& key, const Value& value) {
    auto key_id = relational_model.get_string_id(key, true).id;
    auto value_id = relational_model.get_value_id(value, true).id;

    // catalog.add_node_key(graph_id, key_id);
    return RecordFactory::get(
        node_id,
        key_id,
        value_id
    );
}


Record<3> RelationalGraph::get_record_for_edge_property(uint64_t edge_id, const string& key, const Value& value) {
    auto key_id = relational_model.get_string_id(key, true).id;
    auto value_id = relational_model.get_value_id(value, true).id;

    // catalog.add_edge_key(graph_id, key_id);
    return RecordFactory::get(
        edge_id,
        key_id,
        value_id
    );
}

void RelationalGraph::set_node_properties_stats(uint64_t property_count, map<uint64_t, pair<uint64_t, uint64_t>> m) {
    catalog.set_node_properties_stats(graph_id, property_count, move(m));
}


void RelationalGraph::set_edge_properties_stats(uint64_t property_count, map<uint64_t, pair<uint64_t, uint64_t>> m) {
    catalog.set_edge_properties_stats(graph_id, property_count, move(m));
}
