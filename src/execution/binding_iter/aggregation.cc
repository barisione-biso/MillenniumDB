#include "aggregation.h"

Aggregation::Aggregation(std::unique_ptr<BindingIter>          child_iter,
                         std::map<VarId, std::unique_ptr<Agg>> aggregates,
                         std::vector<VarId>                    group_vars) :
    child_iter (std::move(child_iter)),
    aggregates (std::move(aggregates)),
    group_vars (std::move(group_vars)) { }


void Aggregation::begin(std::ostream& os) {
    child_iter->begin(os);
    saved_next = child_iter->next();

    // reserve space for saved_result
    uint_fast32_t max_var_id = 0;
    for (auto var_id : group_vars) {
        if (var_id.id > max_var_id) {
            max_var_id = var_id.id;
        }
    }
    for (auto&& [var_id, agg] : aggregates) {
        agg->set_binding_iter(child_iter.get());
    }
    saved_result.resize(max_var_id + 1);
}


bool Aggregation::next() {
    if (saved_next) {
        // remember group
        for (auto var_id : group_vars) {
            saved_result[var_id.id] = (*child_iter)[var_id];
        }
        for (auto&& [var_id, agg] : aggregates) {
            agg->begin();
            agg->process();
        }
    } else {
        return false;
    }

    saved_next = false;
    while (child_iter->next()) {
        bool same_group = true;
        // check if group is changed
        for (auto var_id : group_vars) {
            if ( (*child_iter)[var_id] != saved_result[var_id.id] ) {
                same_group = false;
            }
        }
        if (same_group) {
            for (auto&& [var_id, agg] : aggregates) {
                agg->process();
            }
        } else {
            saved_next = true;
            break;
        }
    }
    return true;
}


GraphObject Aggregation::operator[](VarId var_id) const {
    auto search = aggregates.find(var_id);
    if (search != aggregates.end()) {
        return search->second->get();
    } else {
        return (*child_iter)[var_id];
    }
}


void Aggregation::analyze(std::ostream& os, int indent) const {
    child_iter->analyze(os, indent);
    os << std::string(indent, ' ');
    // os << "Aggregation ( checked: " << checked << ", found: " << results << " )\n";
}
