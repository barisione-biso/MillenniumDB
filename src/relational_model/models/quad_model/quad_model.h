#ifndef RELATIONAL_MODEL__QUAD_MODEL_H_
#define RELATIONAL_MODEL__QUAD_MODEL_H_

#include <memory>
#include <type_traits>

// #include "relational_model/cache/strings_cache.h"
#include "relational_model/models/graph_model.h"
#include "relational_model/models/quad_model/quad_catalog.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/hash_table/extendible_hash.h"
#include "storage/index/object_file/object_file.h"
#include "storage/index/random_access_table/random_access_table.h"

class QuadModel : public GraphModel {
public:
    // TODO: extract to relational model?
    static constexpr auto MAX_INLINED_BYTES =  7; // Ids have 8 bytes, 1 for type and 7 remaining
    static constexpr auto TYPE_OFFSET       = 56; // total_bits - bits_for_type: 64 - 8

    static constexpr uint64_t TYPE_MASK                 = 0xFF'00000000000000UL;
    static constexpr uint64_t VALUE_MASK                = 0x00'FFFFFFFFFFFFFFUL;

    // TYPES
    static constexpr uint64_t IDENTIFIABLE_NODE_MASK    = 0x00'00000000000000UL;
    static constexpr uint64_t INLINED_ID_NODE_MASK      = 0x01'00000000000000UL;
    static constexpr uint64_t ANONYMOUS_NODE_MASK       = 0x02'00000000000000UL;
    static constexpr uint64_t CONNECTION_MASK           = 0x03'00000000000000UL;

    static constexpr uint64_t VALUE_EXTERNAL_STR_MASK   = 0x04'00000000000000UL;
    static constexpr uint64_t VALUE_INLINE_STR_MASK     = 0x05'00000000000000UL;

    static constexpr uint64_t VALUE_NEGATIVE_INT_MASK   = 0x06'00000000000000UL;
    static constexpr uint64_t VALUE_POSITIVE_INT_MASK   = 0x07'00000000000000UL;

    static_assert(VALUE_NEGATIVE_INT_MASK < VALUE_POSITIVE_INT_MASK,
        "INLINED INTEGERS WON'T BE ORDERED PROPERTLY IN THE BPT.");

    // TODO: big numbers not supported yet
    // static constexpr uint64_t VALUE_EXTERN_POS_INT_MASK = 0x08'00000000000000UL;
    // static constexpr uint64_t VALUE_EXTERN_NEG_INT_MASK = 0x09'00000000000000UL;

    static constexpr uint64_t VALUE_FLOAT_MASK          = 0x0A'00000000000000UL;
    static constexpr uint64_t VALUE_BOOL_MASK           = 0x0B'00000000000000UL;

    QuadModel(const std::string& db_folder, const int buffer_pool_size);
    ~QuadModel();

    std::unique_ptr<BindingIter> exec(OpSelect&) override;
    std::unique_ptr<BindingIter> exec(manual_plan_ast::Root&) override;
    std::shared_ptr<GraphObject> get_graph_object(ObjectId) override;

    // returns an ID with mask
    ObjectId get_string_id(const std::string& str, bool create_if_not_exists = false);

    // returns an ID with mask
    ObjectId get_value_id(const Value& value, bool create_if_not_exists = false);


    inline QuadCatalog&    catalog()      { return reinterpret_cast<QuadCatalog&>(catalog_buf); }
    inline ObjectFile&     object_file()  { return reinterpret_cast<ObjectFile&>(object_file_buf); }
    inline ExtendibleHash& strings_hash() { return reinterpret_cast<ExtendibleHash&>(strings_cache_buf); }
    // TODO: measure if using strins cache improves times
    // std::unique_ptr<StringsCache>   strings_cache;

    std::unique_ptr<RandomAccessTable<1>> node_table;
    std::unique_ptr<RandomAccessTable<3>> edge_table;

    std::unique_ptr<BPlusTree<2>>   node_label;
    std::unique_ptr<BPlusTree<2>>   label_node;

    std::unique_ptr<BPlusTree<3>>   object_key_value;
    std::unique_ptr<BPlusTree<3>>   key_value_object;

    std::unique_ptr<BPlusTree<4>>   from_to_type_edge;
    std::unique_ptr<BPlusTree<4>>   to_type_from_edge;
    std::unique_ptr<BPlusTree<4>>   type_from_to_edge;

    // special cases
    // std::unique_ptr<BPlusTree<3>>   equal_from_to;      // from=to - type - edge
    // std::unique_ptr<BPlusTree<3>>   equal_from_type;    // from=type - to - edge
    // std::unique_ptr<BPlusTree<3>>   equal_to_type;      // to=type - from - edge
    // std::unique_ptr<BPlusTree<2>>   equal_from_to_type; // from=to=type  -  edge

private:
    uint64_t get_external_id(const std::string& str, bool create_if_not_exists = false);

    typename std::aligned_storage<sizeof(QuadCatalog), alignof(QuadCatalog)>::type               catalog_buf;
    typename std::aligned_storage<sizeof(ObjectFile), alignof(ObjectFile)>::type         object_file_buf;
    typename std::aligned_storage<sizeof(ExtendibleHash), alignof(ExtendibleHash)>::type strings_cache_buf;
};

#endif // RELATIONAL_MODEL__QUAD_MODEL_H_
