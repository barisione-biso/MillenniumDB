#ifndef RELATIONAL_MODEL__GRAPH_H_
#define RELATIONAL_MODEL__GRAPH_H_

#include <memory>

#include "base/graph/node.h"
#include "base/graph/key.h"
#include "base/graph/label.h"
#include "base/graph/edge.h"
#include "file/index/object_file/object_file.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include "relational_model/graph/object_id.h"

class Value;
class Property;
class GraphElement;
class Config;

class RelationalGraph {
public:
    RelationalGraph(int graph_id, Config& config);
    ~RelationalGraph() = default;

    Node create_node();
    Node create_node(u_int64_t id);

    Edge connect_nodes(Node& from, Node& to);
    Edge connect_nodes(u_int64_t edge_id, Node& from, Node& to);

    void add_label(GraphElement&, Label const&);
    void add_property(GraphElement&, Property const&);

    Label get_label(u_int64_t id);
    Key get_key(u_int64_t id);
    unique_ptr<Value> get_value(u_int64_t id);

    ObjectId get_label_id(Label const&);
    ObjectId get_key_id(Key const&);
    ObjectId get_value_id(Key const&);

    unique_ptr<BPlusTree> label2element; // Label|Element
    unique_ptr<BPlusTree> element2label; // Element|Label

    unique_ptr<BPlusTree> element2prop;  // Element|Key|Value
    unique_ptr<BPlusTree> prop2element;  // Key|Value|Element

    unique_ptr<BPlusTree> from_to_edge;  // NodeFrom|NodeTo|Edge
    unique_ptr<BPlusTree> to_from_edge;  // NodeTo|NodeFrom|Edge

    const int graph_id;
    Config& config;

private:
    unique_ptr<BPlusTreeParams> bpt_params_label2element;
    unique_ptr<BPlusTreeParams> bpt_params_element2label;
    unique_ptr<BPlusTreeParams> bpt_params_element2prop;
    unique_ptr<BPlusTreeParams> bpt_params_prop2element;
    unique_ptr<BPlusTreeParams> bpt_params_from_to_edge;
    unique_ptr<BPlusTreeParams> bpt_params_to_from_edge;

    Edge _connect_nodes(u_int64_t edge_id, Node& from, Node& to);

};

#endif //RELATIONAL_MODEL__GRAPH_H_
