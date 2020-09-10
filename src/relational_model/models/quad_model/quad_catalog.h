#ifndef RELATIONAL_MODEL__QUAD_CATALOG_H_
#define RELATIONAL_MODEL__QUAD_CATALOG_H_

#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "base/ids/graph_id.h"
#include "base/ids/object_id.h"
#include "storage/catalog/catalog.h"

class QuadCatalog : public Catalog {
friend class QuadModel;
friend class BulkImport;
public:
    QuadCatalog(const std::string& filename);
    ~QuadCatalog() = default;

    void save_changes();

    void print();

private:
    uint64_t identifiable_defined_count = 0;
    uint64_t anonymous_defined_count = 0;

    uint64_t identifiable_nodes_count;
    uint64_t anonymous_nodes_count;
    uint64_t connections_count;

    uint64_t label_count;
    // type_count not needed because it's equal to connections_count
    uint64_t properties_count;

    uint64_t distinct_labels;
    uint64_t distinct_types;
    uint64_t distinct_keys;

    // uint64_t equal_from_to_count;
    // uint64_t equal_from_type_count;
    // uint64_t equal_to_type_count;
    // uint64_t equal_from_to_type_count;

    std::map<uint64_t, uint64_t> label2total_count;
    std::map<uint64_t, uint64_t> type2total_count;
    std::map<uint64_t, uint64_t> key2total_count;

    std::map<uint64_t, uint64_t> key2distinct;
};

#endif // RELATIONAL_MODEL__QUAD_CATALOG_H_
