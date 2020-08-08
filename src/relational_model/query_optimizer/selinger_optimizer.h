#ifndef RELATIONAL_SELINGER_OPTIMIZER_H_
#define RELATIONAL_SELINGER_OPTIMIZER_H_

#include <memory>
#include <vector>

#include "relational_model/binding/binding_id_iter.h"
#include "relational_model/query_optimizer/join_plan/join_plan.h"

class SelingerOptimizer {
public:
    SelingerOptimizer(std::vector<std::unique_ptr<JoinPlan>>&& base_plans, std::vector<std::string> var_names);
    ~SelingerOptimizer();

    std::unique_ptr<BindingIdIter> get_binding_id_iter();

private:
    std::size_t plans_size;
    std::unique_ptr<JoinPlan>** optimal_plans;
    std::vector<std::string> var_names;

    // from n elements choose r, returns how many combinations can be formed
    // returns 0 if r > n although is a math error (necesary to get_index)
    // only works for small numbers
    static uint64_t nCr(uint_fast32_t n, uint_fast32_t r);

    static uint_fast32_t get_index(bool* arr, uint_fast32_t size);
};

#endif // RELATIONAL_SELINGER_OPTIMIZER_H_
