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

    // TODO: maybe I just need intersection vars. only using var_order outside of that range
    // when assigning nulls.
    // TODO: will var_order conmtains variables of the parant node at optionals?
    std::vector<VarId> var_order;

    BindingId* parent_binding;

    const int_fast32_t base_level; // how many variables in local will be set from outside.
                                   // TODO: may change when including optionals?
    int_fast32_t level;

    int_fast32_t enumeration_level;

    bool first_intersection; // to remember the first time next() is called  after it went down a level


    // iters_for_var[i] is a list of (not-null) pointers of iterators for the variable var_order[i].
    std::vector<std::vector<LeapfrogIter*>> iters_for_var;

    std::vector<std::unique_ptr<TupleBuffer>> buffers;
    std::vector<int_fast32_t> buffer_pos;

    void up();
    void down();
    bool find_intersection_for_current_level();
};

#endif // RELATIONAL_MODEL__LEAPFROG_JOIN_H_
