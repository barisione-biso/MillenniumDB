#ifndef RELATIONAL_MODEL__PROJECTION_H_
#define RELATIONAL_MODEL__PROJECTION_H_

#include <memory>
#include <vector>

#include "base/binding/binding_id_iter.h"
#include "base/binding/binding_iter.h"
#include "relational_model/execution/binding/binding_select.h"

class Select : public BindingIter {

private:
    std::unique_ptr<BindingIter> child_iter;
    uint_fast32_t limit;
    uint_fast32_t count = 0;
    BindingSelect my_binding;

public:
    Select(std::unique_ptr<BindingIter> child_iter, std::vector<std::pair<std::string, VarId>> projection_vars, uint_fast32_t limit);
    ~Select();

    Binding& get_binding() override;
    bool next() override;

    // prints execution statistics
    void analyze(int indent = 0) const override;
};

#endif // RELATIONAL_MODEL__PROJECTION_H_
