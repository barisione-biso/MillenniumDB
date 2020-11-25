#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_H_

#include <set>
#include <map>
#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "relational_model/models/quad_model/quad_model.h"

class BindingIter;
class BindingIdIter;
class OpMatch;
class OpFilter;
class OpSelect;
class JoinPlan;

class QueryOptimizer : public OpVisitor {
public:
    QueryOptimizer(QuadModel& model);
    ~QueryOptimizer() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);
    std::unique_ptr<BindingIter> exec(manual_plan::ast::ManualRoot&);

    void visit(const OpSelect&) override;
    void visit(const OpMatch&) override;
    void visit(const OpFilter&) override;
    void visit(const OpConnection&) override;
    void visit(const OpConnectionType&) override;
    void visit(const OpLabel&) override;
    void visit(const OpProperty&) override;
    void visit(const OpGroupBy&) override;
    void visit(const OpOrderBy&) override;
    void visit(const OpTransitiveClosure&) override;
    void visit(const OpUnjointObject&) override;

private:
    QuadModel& model;
    std::unique_ptr<BindingIter> tmp;
    std::map<std::string, VarId> id_map;
    std::vector<query::ast::SelectItem> select_items;
    int_fast32_t id_count = 0;

    VarId get_var_id(const std::string& var_name);
    ObjectId get_value_id(const common::ast::Value& value);

    std::unique_ptr<BindingIdIter> get_greedy_join_plan(
        std::vector<std::unique_ptr<JoinPlan>> base_plans,
        std::size_t binding_size);

};

#endif // RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
