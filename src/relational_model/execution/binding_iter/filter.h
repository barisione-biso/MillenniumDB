#ifndef RELATIONAL_MODEL__FILTER_H_
#define RELATIONAL_MODEL__FILTER_H_

#include "base/binding/binding_iter.h"
#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"
#include "base/graph/graph_object.h"
#include "relational_model/execution/binding/binding_id_iter.h"

#include <set>
#include <map>
#include <memory>

class Condition;

class Filter : public BindingIter {

private:
    std::unique_ptr<BindingIter> iter;
    std::unique_ptr<Condition> condition;

public:
    Filter(std::unique_ptr<BindingIter> iter, std::unique_ptr<Condition> condition);
    ~Filter() = default;

    void begin();
    std::unique_ptr<Binding> next();

    // prints execution statistics
    void analyze(int indent = 0) const override;
};

#endif // RELATIONAL_MODEL__FILTER_H_
