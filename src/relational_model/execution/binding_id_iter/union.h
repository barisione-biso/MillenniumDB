#ifndef RELATIONAL_MODEL__UNION_H_
#define RELATIONAL_MODEL__UNION_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"

class Union : public BindingIdIter {
public:
    Union(std::size_t binding_size, std::vector<std::unique_ptr<BindingIdIter>> iters);
    ~Union() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    void reset() override;
    bool next() override;

private:
    std::vector<std::unique_ptr<BindingIdIter>> iters;
    std::vector<BindingId*> my_inputs;
    uint_fast32_t current_iter;
};

#endif // RELATIONAL_MODEL__UNION_H_
