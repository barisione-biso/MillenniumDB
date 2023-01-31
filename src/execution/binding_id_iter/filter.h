#pragma once

#include <vector>
#include <memory>

#include "base/binding/binding_id_iter.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

class Filter : public BindingIdIter {
private:
    BindingId* parent_binding;
    std::unique_ptr<BindingIdIter> child;
    std::vector<std::unique_ptr<BindingIdExpr>> filters;

    // statistics
    uint_fast32_t filtered_results = 0;
    uint_fast32_t passed_results = 0;

public:
    Filter(std::unique_ptr<BindingIdIter> child, std::vector<std::unique_ptr<BindingIdExpr>> filters) :
        child   (std::move(child)),
        filters (std::move(filters)) { }

    void analyze(std::ostream& os, int indent = 0) const override;
    void begin(BindingId& parent_binding) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;
};
