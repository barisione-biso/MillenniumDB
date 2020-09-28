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

    BindingId* my_input;
    std::unique_ptr<BindingId> my_binding;

public:
    NodeTableEnum(const VarId var_id, RandomAccessTable<1>& table);
    ~NodeTableEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& input) override;
    void reset(BindingId& input) override;
    BindingId* next() override;
};

#endif // RELATIONAL_MODEL__NODE_TABLE_ENUM_H_
