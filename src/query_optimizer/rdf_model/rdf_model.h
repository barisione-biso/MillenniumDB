#pragma once

#include <type_traits>

#include "execution/graph_model.h"
#include "query_optimizer/rdf_model/rdf_catalog.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/hash/object_file_hash/object_file_hash.h"
#include "storage/index/object_file/object_file.h"
#include "storage/index/random_access_table/random_access_table.h"

class RdfModel : public GraphModel {
    class Destroyer {
        friend class RdfModel;

    private:
        Destroyer() = default;

    public:
        ~Destroyer();
    };

public:
    std::unique_ptr<BPlusTree<3>> spo; // (subject,    predicate, object)
    std::unique_ptr<BPlusTree<3>> pos; // (predicate, object,     subject)
    std::unique_ptr<BPlusTree<3>> osp; // (object,    subject,    predicate)
    std::unique_ptr<BPlusTree<3>> pso; // (predicate, subject,    object)

    // Special cases
    std::unique_ptr<BPlusTree<1>> equal_spo; // (subject=predicate=object)
    std::unique_ptr<BPlusTree<2>> equal_sp;  // (subject=predicate, object)
    std::unique_ptr<BPlusTree<2>> equal_so;  // (subject=object,    predicate)
    std::unique_ptr<BPlusTree<2>> equal_po;  // (predicate=object,  subject)

    std::unique_ptr<BPlusTree<2>> equal_sp_inverted;  // (object,    subject=predicate)
    std::unique_ptr<BPlusTree<2>> equal_so_inverted;  // (predicate, subject=object)
    std::unique_ptr<BPlusTree<2>> equal_po_inverted;  // (subject,   predicate=object)
    

    // necesary to be called before first usage
    static RdfModel::Destroyer init(const std::string& db_folder,
                                    uint_fast32_t      shared_buffer_pool_size,
                                    uint_fast32_t      private_buffer_pool_size,
                                    uint_fast32_t      max_threads);

    std::unique_ptr<BindingIter> exec(Op&, ThreadInfo*) const override;

    ObjectId get_object_id(const GraphObject&) const override;

    GraphObject get_graph_object(ObjectId) const override;

    // Methods used by bulk_import
    // uint64_t get_or_create_object_id(const GraphObject&);

    inline RdfCatalog& catalog() const noexcept {
        return const_cast<RdfCatalog&>(reinterpret_cast<const RdfCatalog&>(catalog_buf));
    }

    inline ObjectFile& object_file() const noexcept {
        return const_cast<ObjectFile&>(reinterpret_cast<const ObjectFile&>(object_file_buf));
    }

    inline ObjectFileHash& strings_hash() const noexcept {
        return const_cast<ObjectFileHash&>(reinterpret_cast<const ObjectFileHash&>(strings_cache_buf));
    }

private:
    typename std::aligned_storage<sizeof(RdfCatalog), alignof(RdfCatalog)>::type         catalog_buf;
    typename std::aligned_storage<sizeof(ObjectFile), alignof(ObjectFile)>::type         object_file_buf;
    typename std::aligned_storage<sizeof(ObjectFileHash), alignof(ObjectFileHash)>::type strings_cache_buf;

    RdfModel(const std::string& db_folder,
             uint_fast32_t      shared_buffer_pool_size,
             uint_fast32_t      private_buffer_pool_size,
             uint_fast32_t      max_threads);

    ~RdfModel();
};

extern RdfModel& rdf_model; // global object
