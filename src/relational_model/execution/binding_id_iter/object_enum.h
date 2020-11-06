#ifndef RELATIONAL_MODEL__OBJECT_ENUM_H_
#define RELATIONAL_MODEL__OBJECT_ENUM_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"

class ObjectEnum : public BindingIdIter {
private:
    const VarId var_id;
    const uint64_t mask;
    const uint64_t max_count;
    uint64_t current_node = 0;

    BindingId* my_input;

public:
    ObjectEnum(std::size_t binding_size, VarId var_id, const uint64_t mask, const uint64_t max_count);
    ~ObjectEnum() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__OBJECT_ENUM_H_
