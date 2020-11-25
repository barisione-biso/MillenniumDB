#ifndef RELATIONAL_MODEL__MATCH_H_
#define RELATIONAL_MODEL__MATCH_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "base/binding/binding_id_iter.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_materialize_id.h"

class Match : public BindingIter {
public:
    Match(GraphModel& model, std::unique_ptr<BindingIdIter> root, size_t binding_size);
    ~Match() = default;

    Binding& get_binding() override;
    bool next() override;

    // prints execution statistics
    void analyze(int indent = 0) const override;

private:
    GraphModel& model;
    std::unique_ptr<BindingIdIter> root;
    BindingId input;
    BindingId* binding_id_root;

    BindingMaterializeId my_binding;
};

#endif // RELATIONAL_MODEL__MATCH_H_
