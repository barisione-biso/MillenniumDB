#ifndef RELATIONAL_MODEL__GROUP_BY_H_
#define RELATIONAL_MODEL__GROUP_BY_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "base/binding/binding_id.h"
#include "relational_model/execution/binding/binding_group_by.h"
#include "relational_model/execution/binding_iter/order_by.h"
#include "base/ids/var_id.h"
#include "base/graph/graph_model.h"
#include "storage/file_id.h"
#include "storage/tuple_collection/tuple_collection.h"

class GroupBy : public BindingIter {
public:
    GroupBy(
        GraphModel& model,
        std::unique_ptr<BindingIter> child,
        std::vector<std::pair<std::string,VarId>> group_vars,
        std::size_t binding_size
        );
    ~GroupBy() = default;
    Binding& get_binding() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    OrderBy order_child;
    std::size_t binding_size;
    BindingGroupBy my_binding;
    FileId group_file_id;
    std::unique_ptr<TupleCollection> group_run;
    uint_fast64_t n_pages = 0;
    uint64_t page_position = 0;
    uint_fast64_t current_page = 0;
};

template class std::unique_ptr<GroupBy>;

#endif // RELATIONAL_MODEL__GROUP_BY_H_
