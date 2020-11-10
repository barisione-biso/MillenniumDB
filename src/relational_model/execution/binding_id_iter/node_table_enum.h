#ifndef RELATIONAL_MODEL__NODE_TABLE_ENUM_H_
#define RELATIONAL_MODEL__NODE_TABLE_ENUM_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"
#include "storage/index/random_access_table/random_access_table.h"

class NodeTableEnum : public BindingIdIter {
private:
    const VarId var_id;
    RandomAccessTable<1>& table;
    uint_fast32_t current_pos;
    uint64_t results = 0;

    BindingId* my_input;

public:
    NodeTableEnum(std::size_t binding_size, const VarId var_id, RandomAccessTable<1>& table);
    ~NodeTableEnum() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__NODE_TABLE_ENUM_H_