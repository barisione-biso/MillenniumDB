#ifndef RELATIONAL_MODEL__RELATIONAL_MODEL_H_
#define RELATIONAL_MODEL__RELATIONAL_MODEL_H_

#include "base/ids/graph_id.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_dir.h"
#include "storage/index/bplus_tree/bplus_tree_params.h"
#include "storage/index/object_file/object_file.h"

#include <memory>
#include <map>

class RelationalGraph;

class RelationalModel {
public:
    static constexpr auto MAX_INLINED_BYTES = 7; // Ids have 8 bytes, 1 for type and 7 remaining
    static constexpr auto TYPE_OFFSET  = 56; // total_bits - bits_for_type: 64 - 8
    static constexpr auto GRAPH_OFFSET = 40; // total_bits - bits_for_type - bits for graph: 64 - 8 - 16

    static constexpr uint64_t NODE_MASK                 = 0x01'0000'0000000000UL;
    static constexpr uint64_t EDGE_MASK                 = 0x02'0000'0000000000UL;
    static constexpr uint64_t LABEL_MASK                = 0x03'0000'0000000000UL;
    static constexpr uint64_t KEY_MASK                  = 0x04'0000'0000000000UL;
    static constexpr uint64_t VALUE_EXTERNAL_STR_MASK   = 0x05'0000'0000000000UL;
    static constexpr uint64_t VALUE_INLINE_STR_MASK     = 0x06'0000'0000000000UL;
    static constexpr uint64_t VALUE_INT_MASK            = 0x07'0000'0000000000UL;
    static constexpr uint64_t VALUE_FLOAT_MASK          = 0x08'0000'0000000000UL;
    static constexpr uint64_t VALUE_BOOL_MASK           = 0x09'0000'0000000000UL;

    static constexpr uint64_t ELEMENT_MASK              = 0x00'0000'FFFFFFFFFFUL; // for nodes and edges
    static constexpr uint64_t VALUE_MASK                = 0x00'FFFF'FFFFFFFFFFUL;
    // static constexpr uint64_t TYPE_MASK                 = 0x0000'FF'0000000000UL;
    // static constexpr uint64_t GRAPH_MASK                = 0xFFFF'00'0000000000UL;
    static constexpr uint64_t TYPE_MASK                 = 0xFF'0000'0000000000UL;
    static constexpr uint64_t GRAPH_MASK                = 0x00'FFFF'0000000000UL; // ONLY EDGES AND NODES HAVE A GRAPH_MASK

    static constexpr auto object_file_name  = "objects.dat";
    static constexpr auto hash2id_name      = "hash_id";

    // Labels
    static constexpr auto label2node_name = "LN";
    static constexpr auto node2label_name = "NL";

    static constexpr auto label2edge_name = "LE";
    static constexpr auto edge2label_name = "EL";

    // Properties
    static constexpr auto key_value_node_name = "KVN";
    static constexpr auto node_key_value_name = "NKV";
    static constexpr auto key_node_value_name = "KNV";

    static constexpr auto key_value_edge_name = "KVE";
    static constexpr auto edge_key_value_name = "EKV";
    static constexpr auto key_edge_value_name = "KEV";

    // Connections
    static constexpr auto from_to_edge_name = "FTE";
    static constexpr auto to_edge_from_name = "TEF";
    static constexpr auto edge_from_to_name = "EFT";

    RelationalModel();
    ~RelationalModel();

    void init();

    ObjectId get_string_unmasked_id(const std::string& str);
    ObjectId get_value_masked_id(const Value& value);

    ObjectId get_or_create_string_unmasked_id(const std::string& str);
    ObjectId get_or_create_value_masked_id(const Value& value);

    RelationalGraph& create_graph(const std::string& graph_name);
    RelationalGraph& get_graph(GraphId);
    std::shared_ptr<GraphObject> get_graph_object(ObjectId);

    ObjectFile& get_object_file();
    BPlusTree&  get_hash2id_bpt();

    BPlusTree& get_label2node();
    BPlusTree& get_label2edge();
    BPlusTree& get_node2label();
    BPlusTree& get_edge2label();

    BPlusTree& get_key_value_node();
    BPlusTree& get_node_key_value();
    BPlusTree& get_key_node_value();

    BPlusTree& get_key_value_edge();
    BPlusTree& get_edge_key_value();
    BPlusTree& get_key_edge_value();

    BPlusTree& get_from_to_edge();
    BPlusTree& get_to_edge_from();
    BPlusTree& get_edge_from_to();


private:
    std::unique_ptr<ObjectFile> object_file;
    std::unique_ptr<BPlusTree>  hash2id; // ObjectHash|ObjectId.

    std::unique_ptr<BPlusTree>  label2node;
    std::unique_ptr<BPlusTree>  label2edge;
    std::unique_ptr<BPlusTree>  node2label;
    std::unique_ptr<BPlusTree>  edge2label;

    std::unique_ptr<BPlusTree>  key_value_node;
    std::unique_ptr<BPlusTree>  node_key_value;
    std::unique_ptr<BPlusTree>  key_node_value;

    std::unique_ptr<BPlusTree>  key_value_edge;
    std::unique_ptr<BPlusTree>  edge_key_value;
    std::unique_ptr<BPlusTree>  key_edge_value;

    std::unique_ptr<BPlusTree>  from_to_edge;
    std::unique_ptr<BPlusTree>  to_edge_from;
    std::unique_ptr<BPlusTree>  edge_from_to;

    std::map<GraphId, std::unique_ptr<RelationalGraph>> graphs;

    uint64_t get_or_create_external_id(std::unique_ptr< std::vector<unsigned char> > bytes);
    uint64_t get_external_id(std::unique_ptr< std::vector<unsigned char> > bytes);
    uint64_t get_value_mask(const Value& value);
};

extern RelationalModel& relational_model; // global object

static struct RelationalModelInitializer {
    RelationalModelInitializer();
    ~RelationalModelInitializer();
} relational_model_initializer; // static initializer for every translation unit


#endif // RELATIONAL_MODEL__RELATIONAL_MODEL_H_
