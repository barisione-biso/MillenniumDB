#pragma once

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_iter.h"
#include "base/query/var.h"
#include "execution/binding_iter/aggregation/agg.h"

class Aggregation : public BindingIter {
public:
    Aggregation(std::unique_ptr<BindingIter>          child_iter,
                std::map<VarId, std::unique_ptr<Agg>> aggregates,
                std::vector<VarId>                    group_vars);

    void begin(std::ostream&) override;

    bool next() override;

    GraphObject operator[](VarId var_id) const override;

    void analyze(std::ostream&, int indent = 0) const override;

private:
    std::unique_ptr<BindingIter> child_iter;

    std::map<VarId, std::unique_ptr<Agg>> aggregates;

    // may be empty if when the query uses aggregates without any group
    std::vector<VarId> group_vars;

    std::vector<GraphObject> saved_result;

    bool saved_next;
};
