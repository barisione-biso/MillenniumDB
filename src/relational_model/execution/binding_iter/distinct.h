#ifndef RELATIONAL_MODEL__DISTINCT_H_
#define RELATIONAL_MODEL__DISTINCT_H_

#include <map>
#include <memory>

#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_distinct.h"

class Distinct : public BindingIter {
public:
    Distinct(GraphModel& model, std::unique_ptr<BindingIter> child_iter);
    ~Distinct() = default;

    inline Binding& get_binding() noexcept override { return my_binding; }

    void begin() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    GraphModel& model;
    std::unique_ptr<BindingIter> child_iter;
    BindingDistinct my_binding;
};

#endif // RELATIONAL_MODEL__DISTINCT_H_