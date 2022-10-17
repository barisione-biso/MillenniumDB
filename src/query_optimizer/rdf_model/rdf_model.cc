#include "rdf_model.h"

#include <iostream>
#include <new>
#include <set>

#include "base/exceptions.h"
#include "base/graph_object/anonymous_node.h"
#include "base/graph_object/edge.h"
#include "base/graph_object/named_node_external.h"
#include "base/graph_object/path.h"
#include "base/path_printer.h"
#include "execution/binding_id_iter/paths/path_manager.h"
#include "query_optimizer/rdf_model/binding_iter_visitor.h"
#include "query_optimizer/rdf_model/graph_object_visitor.h"
// #include "query_optimizer/quad_model/binding_iter_visitor.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

// PathPrinter* Path::path_printer = nullptr;
// StringManager* StringManager::instance = nullptr;

// memory for the object
static typename std::aligned_storage<sizeof(RdfModel), alignof(RdfModel)>::type rdf_model_buf;
// global object
RdfModel& rdf_model = reinterpret_cast<RdfModel&>(rdf_model_buf);


RdfModel::Destroyer RdfModel::init(const std::string& db_folder,
                                   uint_fast32_t      shared_buffer_pool_size,
                                   uint_fast32_t      private_buffer_pool_size,
                                   uint_fast32_t      max_threads) {
    new (&rdf_model) RdfModel(db_folder, shared_buffer_pool_size, private_buffer_pool_size, max_threads);
    return RdfModel::Destroyer();
}


RdfModel::Destroyer::~Destroyer() {
    rdf_model.~RdfModel();
}


RdfModel::RdfModel(const std::string& db_folder,
                   uint_fast32_t      shared_buffer_pool_size,
                   uint_fast32_t      private_buffer_pool_size,
                   uint_fast32_t      max_threads) {
    FileManager::init(db_folder);
    BufferManager::init(shared_buffer_pool_size, private_buffer_pool_size, max_threads);
    PathManager::init(max_threads);

    new (&catalog())       RdfCatalog("catalog.dat");                // placement new
    new (&object_file()) ObjectFile("object_file.dat");              // placement new
    new (&strings_hash()) ObjectFileHash(object_file(), "str_hash"); // placement new

    Path::path_printer      = &path_manager;
    StringManager::instance = &object_file();

    spo = make_unique<BPlusTree<3>>("spo");
    pos = make_unique<BPlusTree<3>>("pos");
    osp = make_unique<BPlusTree<3>>("osp");
    pso = make_unique<BPlusTree<3>>("pso");

    equal_spo = make_unique<BPlusTree<1>>("equal_spo");
    equal_sp  = make_unique<BPlusTree<2>>("equal_sp");
    equal_so  = make_unique<BPlusTree<2>>("equal_so");
    equal_po  = make_unique<BPlusTree<2>>("equal_po");

    equal_sp_inverted = make_unique<BPlusTree<2>>("equal_sp_inverted");
    equal_so_inverted = make_unique<BPlusTree<2>>("equal_so_inverted");
    equal_po_inverted = make_unique<BPlusTree<2>>("equal_po_inverted");
}


RdfModel::~RdfModel() {
    // Must destroy everything before buffer and file manager
    strings_hash().~ObjectFileHash();
    object_file().~ObjectFile();
    catalog().~RdfCatalog();

    spo.reset();
    pos.reset();
    osp.reset();
    pso.reset();

    equal_spo.reset();
    equal_sp.reset();
    equal_so.reset();
    equal_po.reset();
    
    equal_sp_inverted.reset();
    equal_so_inverted.reset();
    equal_po_inverted.reset();

    path_manager.~PathManager();
    buffer_manager.~BufferManager();
    file_manager.~FileManager();
}


std::unique_ptr<BindingIter> RdfModel::exec(Op& op, ThreadInfo* thread_info) const {
    auto vars = op.get_vars();
    auto query_optimizer = SPARQL::BindingIterVisitor(std::move(vars), thread_info);
    op.accept_visitor(query_optimizer);
    return move(query_optimizer.tmp);
}


GraphObject RdfModel::get_graph_object(ObjectId object_id) const {
    if (object_id.is_not_found()) {
        return GraphObject::make_not_found();
    }
    if (object_id.is_null()) {
        return GraphObject::make_null();
    }
    auto mask        = object_id.id & ObjectId::TYPE_MASK;
    auto unmasked_id = object_id.id & ObjectId::VALUE_MASK;
    switch (mask) {
    case ObjectId::VALUE_EXTERNAL_STR_MASK: {
        return GraphObject::make_string_external(unmasked_id);
    }

    case ObjectId::VALUE_INLINE_STR_MASK: {
        char c[8];
        int  shift_size = 6 * 8;
        for (int i = 0; i < ObjectId::MAX_INLINED_BYTES; i++) {
            uint8_t byte = (object_id.id >> shift_size) & 0xFF;
            c[i]         = byte;
            shift_size -= 8;
        }
        c[7] = '\0';
        return GraphObject::make_string_inlined(c);
    }

    case ObjectId::EXTERNAL_IRI_MASK: {
        return GraphObject::make_iri_external(unmasked_id);
    }

    case ObjectId::INLINE_IRI_MASK: {
        char c[7];
        int  suffix_shift_size = 5 * 8;
        for (int i = 0; i < 6; i++) {
            uint8_t byte = (object_id.id >> suffix_shift_size) & 0xFF;
            c[i]         = byte;
            suffix_shift_size -= 8;
        }
        c[6] = '\0';
        int     prefix_shift_size = 6 * 8;
        uint8_t prefix_id         = (object_id.id & 0x00FF'0000'0000'0000UL) >> prefix_shift_size;
        return GraphObject::make_iri_inlined(c, prefix_id);
    }

    case ObjectId::VALUE_INLINE_STR_DATATYPE_MASK: {
        char c[6];
        int  shift_size = 4 * 8;
        for (int i = 0; i < 5; i++) {
            uint8_t byte = (object_id.id >> shift_size) & 0xFF;
            c[i]         = byte;
            shift_size -= 8;
        }
        c[5] = '\0';
        int prefix_shift_size = 5 * 8;
        uint16_t datatype_id = (object_id.id & 0x00FF'FF00'0000'0000UL) >> prefix_shift_size;
        return GraphObject::make_literal_datatype_inlined(c, datatype_id);
    }

    case ObjectId::VALUE_EXTERNAL_STR_DATATYPE_MASK: {
        return GraphObject::make_literal_datatype_external(unmasked_id);
    }

    case ObjectId::VALUE_INLINE_STR_LANGUAGE_MASK: {
        char c[6];
        int  shift_size = 4 * 8;
        for (int i = 0; i < 5; i++) {
            uint8_t byte = (object_id.id >> shift_size) & 0xFF;
            c[i]         = byte;
            shift_size -= 8;
        }
        c[5] = '\0';
        int prefix_shift_size = 5 * 8;
        uint16_t language_id = (object_id.id & 0x00FF'FF00'0000'0000UL) >> prefix_shift_size;
        return GraphObject::make_literal_language_inlined(c, language_id);
    }

    case ObjectId::VALUE_EXTERNAL_STR_LANGUAGE_MASK: {
        return GraphObject::make_literal_language_external(unmasked_id);
    }

    case ObjectId::ANONYMOUS_NODE_MASK: {
        return GraphObject::make_anonymous(unmasked_id);
    }

    case ObjectId::VALUE_DATETIME_MASK: {
        return GraphObject::make_datetime(unmasked_id);
    }

    case ObjectId::VALUE_DECIMAL_MASK: {
        return GraphObject::make_decimal(unmasked_id);
    }

    case ObjectId::VALUE_BOOLEAN_MASK: {
        return GraphObject::make_boolean(unmasked_id ? true : false);
    }

    default: {
        throw LogicException("Unhandled Object Type.");
    }
    }
}


ObjectId RdfModel::get_object_id(const GraphObject& graph_object) const {
    SPARQL::GraphObjectVisitor visitor(false);
    return visitor(graph_object);
}


// uint64_t RdfModel::get_or_create_object_id(const GraphObject& graph_object) {
//     return std::visit(GraphObjectVisitor(true), graph_object.value).id;
//     GraphObjectVisitor visitor(true);
//     return visitor(graph_object).id;
//     TODO: Connect graph object visitor
//     return 0;
// }
