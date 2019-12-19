#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_H_

#include <string>
#include <vector>
#include <memory>

#include "relational_model/physical_plan/binding_id_iter/binding_id_iter.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizer {
public:
    QueryOptimizer();
    ~QueryOptimizer() = default;
    unique_ptr<BindingIdIter> get_query_plan(std::vector<QueryOptimizerElement*> elements);
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
