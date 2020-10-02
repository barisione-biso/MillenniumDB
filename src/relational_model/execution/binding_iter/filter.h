#ifndef RELATIONAL_MODEL__FILTER_H_
#define RELATIONAL_MODEL__FILTER_H_

#include <set>
#include <map>
#include <memory>

#include "base/binding/binding_iter.h"
#include "base/ids/var_id.h"
#include "base/graph/graph_object.h"
#include "relational_model/execution/binding/binding_id_iter.h"
#include "relational_model/models/graph_model.h"

class Condition;

class Filter : public BindingIter {

private:
    GraphModel& model;
    std::unique_ptr<BindingIter> iter;
    std::unique_ptr<Condition> condition;
    uint64_t results = 0;

public:
    Filter(GraphModel& model, std::unique_ptr<BindingIter> iter, std::unique_ptr<Condition> condition);
    ~Filter() = default;

    void begin();
    std::unique_ptr<Binding> next();

    // prints execution statistics
    void analyze(int indent = 0) const override;
};

#endif // RELATIONAL_MODEL__FILTER_H_
