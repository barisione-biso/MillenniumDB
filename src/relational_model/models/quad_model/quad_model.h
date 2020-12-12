#ifndef RELATIONAL_MODEL__QUAD_MODEL_H_
#define RELATIONAL_MODEL__QUAD_MODEL_H_

#include <memory>
#include <type_traits>

#include "base/graph/graph_model.h"
#include "relational_model/models/quad_model/quad_catalog.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/hash_table/extendible_hash.h"
#include "storage/index/object_file/object_file.h"
#include "storage/index/random_access_table/random_access_table.h"

class QuadModel : public GraphModel {
public:
    QuadModel(const std::string& db_folder, const int buffer_pool_size);
    ~QuadModel();

    std::unique_ptr<BindingIter> exec(OpSelect&) override;
    std::unique_ptr<BindingIter> exec(manual_plan::ast::ManualRoot&) override;
    ObjectId get_object_id(const GraphObject& obj) override;
    ObjectId get_object_id(const GraphObject& obj, bool create_if_not_exists);

    GraphObject get_graph_object(ObjectId) override;
    GraphObject get_property_value(GraphObject& var, const ObjectId key) override;

    // returns an ID with mask
    ObjectId get_string_id(const std::string& str, bool create_if_not_exists = false);
    ObjectId get_identifiable_object_id(const std::string& str, bool create_if_not_exists = false);
    uint64_t get_external_id(const std::string& str, bool create_if_not_exists = false);

    inline QuadCatalog&    catalog()      noexcept { return reinterpret_cast<QuadCatalog&>(catalog_buf); }
    inline ObjectFile&     object_file()  noexcept { return reinterpret_cast<ObjectFile&>(object_file_buf); }
    inline ExtendibleHash& strings_hash() noexcept { return reinterpret_cast<ExtendibleHash&>(strings_cache_buf); }

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
    std::unique_ptr<BPlusTree<3>>   equal_from_to;      // from=to - type - edge
    std::unique_ptr<BPlusTree<3>>   equal_from_type;    // from=type - to - edge
    std::unique_ptr<BPlusTree<3>>   equal_to_type;      // to=type - from - edge
    std::unique_ptr<BPlusTree<2>>   equal_from_to_type; // from=to=type  -  edge

    std::unique_ptr<BPlusTree<3>>   equal_from_to_inverted;
    std::unique_ptr<BPlusTree<3>>   equal_from_type_inverted;
    std::unique_ptr<BPlusTree<3>>   equal_to_type_inverted;

private:
    typename std::aligned_storage<sizeof(QuadCatalog), alignof(QuadCatalog)>::type       catalog_buf;
    typename std::aligned_storage<sizeof(ObjectFile), alignof(ObjectFile)>::type         object_file_buf;
    typename std::aligned_storage<sizeof(ExtendibleHash), alignof(ExtendibleHash)>::type strings_cache_buf;
};

#endif // RELATIONAL_MODEL__QUAD_MODEL_H_
