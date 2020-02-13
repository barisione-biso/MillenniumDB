#ifndef RELATIONAL_MODEL__GRAPH_H_
#define RELATIONAL_MODEL__GRAPH_H_

#include "base/ids/graph_id.h"
#include "base/graph/node.h"
#include "base/graph/key.h"
#include "base/graph/label.h"
#include "base/graph/edge.h"
#include "file/index/object_file/object_file.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include "base/ids/object_id.h"

#include <memory>
#include <string>


class Value;
class Property;

class RelationalGraph {
public:
    const GraphId graph_id;

    std::unique_ptr<BPlusTree> label2node;
    std::unique_ptr<BPlusTree> label2edge;
    std::unique_ptr<BPlusTree> node2label;
    std::unique_ptr<BPlusTree> edge2label;

    std::unique_ptr<BPlusTree> prop2node;
    std::unique_ptr<BPlusTree> prop2edge;
    std::unique_ptr<BPlusTree> node2prop;
    std::unique_ptr<BPlusTree> edge2prop;

    std::unique_ptr<BPlusTree> from_to_edge;
    std::unique_ptr<BPlusTree> to_edge_from;
    std::unique_ptr<BPlusTree> edge_from_to;

    RelationalGraph(GraphId graph_id);
    ~RelationalGraph() = default;

    uint64_t create_node();
    uint64_t create_edge();

    //////////////////////////////  IDS RECEIVED BY THESE METHODS ARE UNMASKED  //////////////////////////////
                                                                                                            //
    // Methods used by bulk import:                                                                         //
    Record get_record_for_connection(uint64_t node_from_id, uint64_t node_to_id, uint64_t edge_id);         //
                                                                                                            //
    Record get_record_for_node_label(uint64_t node_id, const std::string& label);                           //
    Record get_record_for_edge_label(uint64_t edge_id, const std::string& label);                           //
                                                                                                            //
    Record get_record_for_node_property(uint64_t node_id, const std::string& key, const Value& value);      //
    Record get_record_for_edge_property(uint64_t edge_id, const std::string& key, const Value& value);      //
                                                                                                            //
    // Methods used by one-by-one import:                                                                   //
    void add_label_to_node(uint64_t node_id, const std::string&);                                           //
    void add_label_to_edge(uint64_t edge_id, const std::string&);                                           //
                                                                                                            //
    void add_property_to_node(uint64_t node_id, const std::string& key, const Value& value);                //
    void add_property_to_edge(uint64_t edge_id, const std::string& key, const Value& value);                //
                                                                                                            //
    void connect_nodes(uint64_t id_from, uint64_t id_to, uint64_t id_edge);                                 //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////// IDS RECEIVED BY GET METHODS CAN BE MASKED, THEY WILL BE UNMASKED ////////////////////
                                                                                                            //
    // Node  get_node (uint64_t id);                                                                           //
    // Label get_label(uint64_t id);                                                                           //
    // Key   get_key  (uint64_t id);                                                                           //
    // std::unique_ptr<Value> get_value(uint64_t id);                                                          //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    // uint64_t add_label   (uint64_t id, const string& label);
    // uint64_t add_property(uint64_t id, const string& key, const Value& value);
    // uint64_t get_or_create_id(const std::string& str);
    // uint64_t get_or_create_id(std::unique_ptr< std::vector<char> > obj_bytes);
};

#endif //RELATIONAL_MODEL__GRAPH_H_
