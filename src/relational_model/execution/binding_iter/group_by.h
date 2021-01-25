#ifndef RELATIONAL_MODEL__GROUP_BY_H_
#define RELATIONAL_MODEL__GROUP_BY_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_id.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding_iter/order_by.h"
#include "relational_model/execution/binding/binding_group_by.h"
#include "storage/file_id.h"
#include "storage/tuple_collection/tuple_collection.h"

class GroupBy : public BindingIter {
public:
    GroupBy(GraphModel& model,
            std::unique_ptr<BindingIter> child,
            std::size_t binding_size,
            std::vector<std::pair<std::string,VarId>> _group_vars,
            std::vector<bool> ascending);
    ~GroupBy() = default;

    Binding& get_binding() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    OrderBy order_child;
    std::size_t binding_size;
    std::vector<std::pair<std::string, VarId>> group_vars;
    BindingGroupBy my_binding;
    FileId group_file_id;
    std::vector<GraphObject> current_group_tuple;
    std::vector<GraphObject> current_tuple;
    std::unique_ptr<TupleCollection> group_run;
    uint_fast64_t n_pages = 0;

    bool has_same_group_vars();
    void add_tuple_to_group();
    void compute_agregation_function();
};

template class std::unique_ptr<GroupBy>;

#endif // RELATIONAL_MODEL__GROUP_BY_H_
