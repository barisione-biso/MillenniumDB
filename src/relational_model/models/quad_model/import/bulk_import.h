#ifndef RELATIONAL_MODEL__QUAD_BULK_IMPORT_H_
#define RELATIONAL_MODEL__QUAD_BULK_IMPORT_H_

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <memory>

#include "base/parser/grammar/import/import_ast.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/quad_catalog.h"
#include "storage/index/ordered_file/ordered_file.h"

class BulkImport {
public:
    BulkImport(const std::string& filename, QuadModel& model);
    ~BulkImport() = default;

    void start_import();

private:
    std::ifstream import_file;
    QuadModel& model;
    QuadCatalog& catalog;

    OrderedFile<2> node_labels;             // node_id, label_id
    OrderedFile<3> object_key_value;        // object_id, key_id, value_id
    OrderedFile<4> from_to_type_edge;       // from_id, to_id, type_id, edge_id

    // To create indexes for special cases
    OrderedFile<3> equal_from_to;           // from/to, type, edge
    OrderedFile<3> equal_from_type;         // from/type, to, edge
    OrderedFile<3> equal_to_type;           // to/type, from, edge
    OrderedFile<2> equal_from_to_type;      // from/to/type,  edge

    // TODO: maybe I should use disk in case this is too big?
    std::map<std::string, uint64_t> identificable_node_dict;
    std::map<std::string, uint64_t> anonymous_node_dict;

    uint64_t process_node(const import::ast::Node node);
    uint64_t process_edge(const import::ast::Edge edge);
    uint64_t process_implicit_edge(const import::ast::ImplicitEdge implicit_edge, const uint64_t implicit_object_id);

    uint64_t create_connection(const uint64_t from_id, const uint64_t to_id, const uint64_t type_id);

    uint64_t get_node_id(const std::string& node_name);
    uint64_t get_anonymous_node_id(const std::string& tmp_name);

    template <std::size_t N>
    void set_distinct_type_stats(OrderedFile<N>& ordered_file, std::map<uint64_t, uint64_t>& m);

    // template <std::size_t N>
    // void merge_tree_and_ordered_file(std::unique_ptr<BPlusTree<N>>&, OrderedFile<N>&);

    // void set_property_stats(std::map<uint64_t, std::pair<uint64_t, uint64_t>>& m,
    //                         OrderedFile<3>& ordered_properties);
};

#endif // RELATIONAL_MODEL__QUAD_BULK_IMPORT_H_