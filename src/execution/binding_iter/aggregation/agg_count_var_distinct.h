#pragma once

#include <set>

#include "execution/binding_iter/aggregation/agg.h"
// #include "storage/index/hash/distinct_binding_hash/distinct_binding_hash.h"

class AggCountVarDistinct : public Agg {
public:
    AggCountVarDistinct(VarId var_id) : var_id (var_id) { }

    void begin() override {
        // extendable_table = std::make_unique<DistinctBindingHash<GraphObject>>(1);
        count = 0;
        distinct_set.clear();
        // tuple = { GraphObject::make_null() };
    }

    void process() override;

    // indicates the end of a group
    GraphObject get() override {
        return GraphObject::make_int(count);
    }

private:
    VarId var_id;

    int64_t count;

    // std::vector<GraphObject> tuple;

    // std::unique_ptr<DistinctBindingHash<GraphObject>> extendable_table;
    std::set<GraphObject> distinct_set;
};
