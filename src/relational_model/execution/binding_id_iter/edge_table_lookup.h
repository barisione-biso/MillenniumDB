#ifndef RELATIONAL_MODEL__EDGE_TABLE_LOOKUP_H_
#define RELATIONAL_MODEL__EDGE_TABLE_LOOKUP_H_

#include <memory>
#include <vector>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "storage/index/random_access_table/random_access_table.h"

class EdgeTableLookup : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    RandomAccessTable<3>& table;
    VarId edge;
    Id from;
    Id to;
    Id type;
    uint64_t lookups = 0;
    uint64_t results = 0;
    bool already_looked;

    BindingId* parent_binding;

public:
    EdgeTableLookup(std::size_t binding_size, RandomAccessTable<3>& table, VarId edge, Id from, Id to, Id type);
    ~EdgeTableLookup() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;
};

#endif // RELATIONAL_MODEL__NODE_TABLE_ENUM_H_
