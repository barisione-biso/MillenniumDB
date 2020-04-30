#ifndef RELATIONAL_MODEL__NAMED_GRAPH_VAR_H_
#define RELATIONAL_MODEL__NAMED_GRAPH_VAR_H_

#include "base/ids/graph_id.h"
#include "relational_model/relational_model.h"
#include "relational_model/physical_plan/binding_id_iter/scan_ranges/scan_range.h"

class NamedGraphVar : public ScanRange {
private:
    VarId var_id;
    uint64_t graph_prefix;

public:
    NamedGraphVar(VarId var_id, GraphId graph_id)
        : var_id(var_id)
    {
        graph_prefix = graph_id << RelationalModel::GRAPH_OFFSET;
    }

    uint64_t get_min(BindingId&) override {
        return graph_prefix;
    }

    uint64_t get_max(BindingId&) override {
        return graph_prefix | 0xFF'0000'FFFFFFFFFFUL;
    }

    void try_assign(BindingId& binding, ObjectId obj_id) override {
        binding.add(var_id, obj_id);
    }
};

#endif // RELATIONAL_MODEL__NAMED_GRAPH_VAR_H_
