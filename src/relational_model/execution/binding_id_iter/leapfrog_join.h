#ifndef RELATIONAL_MODEL__LEAPFROG_JOIN_H_
#define RELATIONAL_MODEL__LEAPFROG_JOIN_H_

#include <memory>
#include <vector>

#include "base/binding/binding_id_iter.h"
#include "base/ids/var_id.h"
#include "storage/index/bplus_tree/leapfrog_iter.h"

class LeapfrogJoin : public BindingIdIter {
public:
    LeapfrogJoin(std::vector<std::unique_ptr<LeapfrogIter>> leapfrog_iters,
                 std::vector<VarId> var_order);
    ~LeapfrogJoin() = default;

    void analyze(int indent = 0) const override;
    void begin(BindingId& parent_binding, bool parent_has_next) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;

private:
    std::vector<std::unique_ptr<LeapfrogIter>> leapfrog_iters;

    // TODO: will var_order contain variables of the parant node at optionals?
    std::vector<VarId> var_order; // contains variables from intersection_vars and enumeration_vars

    BindingId* parent_binding;

    const int_fast32_t base_level; // how many variables in local will be set from outside.
                                   // TODO: may change when including optionals?

    int_fast32_t level; // level starts at base_level, and will vary between [base_level, enumeration_level]
                        // when level-(base_level-1) means there is no more tuples left

    int_fast32_t enumeration_level;

    // iters_for_var[i] is a list of (not-null) pointers of iterators for the variable var_order[i].
    std::vector<std::vector<LeapfrogIter*>> iters_for_var;

    std::vector<std::unique_ptr<TupleBuffer>> buffers;
    std::vector<int_fast32_t> buffer_pos;

    void up();
    void down();
    bool find_intersection_for_current_level();
};

#endif // RELATIONAL_MODEL__LEAPFROG_JOIN_H_
