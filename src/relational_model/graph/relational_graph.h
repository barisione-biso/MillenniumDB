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
    const int graph_id;
    Config& config;

    unique_ptr<BPlusTree> label2element; // Label|Element
    unique_ptr<BPlusTree> element2label; // Element|Label

    unique_ptr<BPlusTree> element2prop;  // Element|Key|Value
    unique_ptr<BPlusTree> prop2element;  // Key|Value|Element

    unique_ptr<BPlusTree> from_to_edge;  // NodeFrom|NodeTo|Edge
    unique_ptr<BPlusTree> to_from_edge;  // NodeTo|NodeFrom|Edge

    RelationalGraph(int graph_id, Config& config);
    ~RelationalGraph();

    uint64_t create_node();
    uint64_t create_edge();

    //////////////////////////////  IDS RECEIVED BY THESE METHODS ARE UNMASKED  //////////////////////////////
                                                                                                            //
    // Methods used by bulk import:                                                                         //
    Record get_record_for_edge      (uint64_t node_from_id, uint64_t node_to_id, uint64_t edge_id);         //
    Record get_record_for_node_label(uint64_t node_id, const string& label);                                //
    Record get_record_for_edge_label(uint64_t edge_id, const string& label);                                //
                                                                                                            //
    Record get_record_for_node_property(uint64_t node_id, const string& key, const Value& value);           //
    Record get_record_for_edge_property(uint64_t edge_id, const string& key, const Value& value);           //
                                                                                                            //
    // Methods used by one-by-one import:                                                                   //
    void add_label_to_node(uint64_t node_id, const string&);                                                //
    void add_label_to_edge(uint64_t edge_id, const string&);                                                //
                                                                                                            //
    void add_property_to_node(uint64_t node_id, const string& key, const Value& value);                     //
    void add_property_to_edge(uint64_t edge_id, const string& key, const Value& value);                     //
                                                                                                            //
    void connect_nodes(uint64_t id_from, uint64_t id_to, uint64_t id_edge);                                 //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////// IDS RECEIVED BY GET METHODS CAN BE MASKED, THEY WILL BE UNMASKED ////////////////////
                                                                                                            //
    Node              get_node (uint64_t id);                                                               //
    Label             get_label(uint64_t id);                                                               //
    Key               get_key  (uint64_t id);                                                               //
    unique_ptr<Value> get_value(uint64_t id);                                                               //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    ObjectId get_label_id(Label const&); // TODO: define what happens if doesnt exist, ¿special ObjectId = 0?
    ObjectId get_key_id  (Key   const&); // TODO: define what happens if doesnt exist, ¿special ObjectId = 0?
    ObjectId get_value_id(Value const&); // TODO: define what happens if doesnt exist, ¿special ObjectId = 0?

private:
    unique_ptr<BPlusTreeParams> bpt_params_label2element;
    unique_ptr<BPlusTreeParams> bpt_params_element2label;
    unique_ptr<BPlusTreeParams> bpt_params_element2prop;
    unique_ptr<BPlusTreeParams> bpt_params_prop2element;
    unique_ptr<BPlusTreeParams> bpt_params_from_to_edge;
    unique_ptr<BPlusTreeParams> bpt_params_to_from_edge;

    uint64_t add_label   (uint64_t id, const string& label);                   // returns masked label_id
    uint64_t add_property(uint64_t id, const string& key, const Value& value); // returns masked key_id
    uint64_t get_or_create_id(unique_ptr< vector<char> > obj_bytes);           // returns unmasked id

    Record get_record_for_element_label   (uint64_t element_id, const string& label);
    Record get_record_for_element_property(uint64_t element_id, const string& key, const Value& value);
};

#endif //RELATIONAL_MODEL__GRAPH_H_
