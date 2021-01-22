#ifndef RELATIONAL_MODEL__ORDER_BY_H_
#define RELATIONAL_MODEL__ORDER_BY_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_id.h"
#include "base/binding/binding_iter.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_id.h"
#include "storage/tuple_collection/tuple_collection.h"

class OrderBy : public BindingIter {
public:
    OrderBy(
        GraphModel& model,
        std::unique_ptr<BindingIter> child,
        std::vector<std::pair<std::string,VarId>> order_vars,
        std::size_t binding_size,
        const bool ascending);
    ~OrderBy() = default;
    Binding& get_binding() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    std::unique_ptr<BindingIter> child;
    std::unique_ptr<TupleCollection> run;
    std::size_t binding_size;
    BindingOrderBy my_binding;
    FileId first_file_id;
    FileId second_file_id;
    std::unique_ptr<MergeOrderedTupleCollection> merger;
    uint_fast64_t n_pages = 0;
    uint64_t page_position = 0;
    uint_fast64_t current_page = 0;
    FileId* output_file_id;
    void mergeSort();
};

template class std::unique_ptr<OrderBy>;

#endif // RELATIONAL_MODEL__ORDER_BY_H_
