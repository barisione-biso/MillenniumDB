#ifndef RELATIONAL_MODEL__ORDER_BY_H_
#define RELATIONAL_MODEL__ORDER_BY_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "base/binding/binding_id_iter.h"
#include "base/binding/binding_id.h"
#include "relational_model/execution/binding/binding_order_by.h"
#include "base/ids/var_id.h"
#include "base/graph/graph_model.h"
#include "storage/file_id.h"
#include "storage/tuple_collection/tuple_collection.h"

class OrderBy : public BindingIter {
public:
    OrderBy(
        std::unique_ptr<BindingIter> root,
        std::vector<std::pair<std::string,VarId>> order_vars,
        size_t binding_size,
        const bool ascending);
    ~OrderBy() = default;

    // BindingId& begin(BindingId& input) override;
    Binding& get_binding() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    std::unique_ptr<BindingIter> root;
    BindingOrderBy my_binding;
    //const bool ascending;
    std::vector<std::pair<std::string, VarId>> order_vars;
};

template class std::unique_ptr<OrderBy>;

#endif // RELATIONAL_MODEL__ORDER_BY_H_
