#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_H_

#include <string>
#include <vector>
#include <memory>

#include "base/binding/binding_iter.h"
#include "relational_model/physical_plan/binding_id_iter.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizer {
private:
    ObjectFile& obj_file;
    std::vector<std::string> names;
    std::vector<int_fast32_t> var_positions;
    int_fast32_t global_vars;

    std::unique_ptr<BindingIdIter> get_join_plan(std::vector<QueryOptimizerElement*> elements);

public:
    QueryOptimizer(ObjectFile&, int_fast32_t global_vars);
    ~QueryOptimizer() = default;
    std::unique_ptr<BindingIter> get_query_plan(std::vector<QueryOptimizerElement*> elements,
        std::map<std::string, std::string> constants, std::vector<std::string> names,
        std::vector<int_fast32_t> var_positions);

};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
