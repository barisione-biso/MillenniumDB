#ifndef RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
#define RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_

#include "base/var/var_id.h"
#include "relational_model/physical_plan/binding_id_iter/binding_id_iter.h"

#include <map>
#include <memory>
#include <vector>

class Config;

class IndexNestedLoopJoin : public BindingIdIter {
public:
    IndexNestedLoopJoin(Config& config, BindingIdIter& left, BindingIdIter& right);
    ~IndexNestedLoopJoin() = default;
    void init(std::shared_ptr<BindingId const> input);
    void reset(std::shared_ptr<BindingId const> input);
    std::unique_ptr<BindingId const> next();

private:
    std::unique_ptr<BindingId const> construct_binding(BindingId const& lhs, BindingId const& rhs);

    Config const& config;
    BindingIdIter& left;
    BindingIdIter& right;

    std::shared_ptr<BindingId const> current_left;
    std::unique_ptr<BindingId const> current_right;

    std::vector<VarId> vars;
};

#endif //RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
