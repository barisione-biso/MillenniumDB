#ifndef RELATIONAL_MODEL__OBJECT_ENUM_H_
#define RELATIONAL_MODEL__OBJECT_ENUM_H_

#include <memory>
#include <vector>

#include "base/binding/binding_id_iter.h"
#include "base/ids/var_id.h"

class ObjectEnum : public BindingIdIter {
private:
    const VarId var_id;
    const uint64_t mask;
    const uint64_t max_count;
    uint64_t current_node = 0;
    uint64_t results = 0;

    BindingId* parent_binding;

public:
    ObjectEnum(VarId var_id, const uint64_t mask, const uint64_t max_count);
    ~ObjectEnum() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;
};

#endif // RELATIONAL_MODEL__OBJECT_ENUM_H_
