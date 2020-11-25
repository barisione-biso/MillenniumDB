#ifndef RELATIONAL_MODEL__JOIN_PLAN_H_
#define RELATIONAL_MODEL__JOIN_PLAN_H_

#include <memory>
#include <variant>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/unassigned_var.h"

// Abstract Class
class JoinPlan {
public:
    using Id = std::variant<VarId, ObjectId>;

    virtual ~JoinPlan() = default;

    virtual double estimate_cost() = 0;
    virtual double estimate_output_size() = 0;

    bool cartesian_product_needed(JoinPlan& other) {
        return (get_vars() & other.get_vars()) == 0;
    }

    // TODO: pensar caso de rangos que vendran por optimización del where con un rango
    // maximo y minimo
    std::unique_ptr<ScanRange> get_scan_range(Id id, bool assigned) {
        if ( std::holds_alternative<ObjectId>(id) ) {
            return std::make_unique<Term>(std::get<ObjectId>(id));
        } else if (assigned) {
            return std::make_unique<AssignedVar>(std::get<VarId>(id));
        } else {
            return std::make_unique<UnassignedVar>(std::get<VarId>(id));
        }
    }

    // TODO: using 64 bits limits the number of variables up to 64
    virtual void set_input_vars(const uint64_t input_vars) = 0;
    virtual uint64_t get_vars() = 0;

    virtual std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) = 0;
    virtual std::unique_ptr<JoinPlan> duplicate() = 0;

    virtual void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) = 0;
};

#endif // RELATIONAL_MODEL__JOIN_PLAN_H_
