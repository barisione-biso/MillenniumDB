#ifndef RELATIONAL_MODEL__UNION_H_
#define RELATIONAL_MODEL__UNION_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_id_iter.h"

class Union : public BindingIdIter {
public:
    Union(std::vector<std::unique_ptr<BindingIdIter>> iters);
    ~Union() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;

private:
    std::vector<std::unique_ptr<BindingIdIter>> iters;
    std::vector<BindingId*> my_inputs;
    uint_fast32_t current_iter;
};

#endif // RELATIONAL_MODEL__UNION_H_
