#ifndef RELATIONAL_MODEL__JOIN_PLAN_H_
#define RELATIONAL_MODEL__JOIN_PLAN_H_

#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "base/binding/binding_id_iter.h"
#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
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
        return (get_vars() & other.get_vars()) == 0; // TODO: cuando hay optionals esto funciona raro
                                                     // Ej: ?x :P1 ?y OPT { ?x :P2 ?a, ?x :P3 ?b }
                                                     // dentro del optional hay un producto cruz pero esta funcion no lo detecta
    }

    // TODO: using 64 bits limits the number of variables up to 64
    // TODO: Change uint64_t to std::set<VarId>
    // virtual void set_input_vars(const std::set<VarId>& input_vars) = 0;
    virtual void set_input_vars(const uint64_t input_vars) = 0;
    virtual uint64_t get_vars() = 0;

    virtual std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) = 0;
    virtual std::unique_ptr<JoinPlan> duplicate() = 0;

    virtual void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) = 0;
};

#endif // RELATIONAL_MODEL__JOIN_PLAN_H_
