#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_H_

#include <memory>
#include <string>
#include <vector>

class BindingIdIter;
class BindingIter;
class ObjectFile;
class Op;
class OpSelect;
class QueryOptimizerElement;

class QueryOptimizer {

private:
    ObjectFile& obj_file;
    std::vector<std::string> names;
    std::vector<int_fast32_t> var_positions;
    int_fast32_t global_vars;
    std::unique_ptr<Op> op;

    std::unique_ptr<BindingIdIter> get_join_plan(std::vector<std::unique_ptr<QueryOptimizerElement>> elements);

public:
    QueryOptimizer(ObjectFile&);
    ~QueryOptimizer() = default;

    std::unique_ptr<BindingIter> get_select_plan(std::unique_ptr<OpSelect> op_select);

};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
