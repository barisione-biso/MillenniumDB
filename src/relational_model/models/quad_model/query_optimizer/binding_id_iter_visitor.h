#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_

#include <map>
#include <set>
#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class BindingIdIter;

class BindingIdIterVisitor : public OpVisitor {
public:
    BindingIdIterVisitor(QuadModel& model);
    ~BindingIdIterVisitor() = default;

    std::unique_ptr<BindingIdIter> exec(OpSelect&);
    std::unique_ptr<BindingIdIter> exec(manual_plan::ast::ManualRoot&);

    void visit(const OpMatch&) override;
    void visit(const OpOptional&) override;
    void visit(const OpSelect&) override;
    void visit(const OpFilter&) override;
    void visit(const OpConnection&) override;
    void visit(const OpConnectionType&) override;
    void visit(const OpLabel&) override;
    void visit(const OpProperty&) override;
    void visit(const OpGroupBy&) override;
    void visit(const OpOrderBy&) override;
    void visit(const OpTransitiveClosure&) override;
    void visit(const OpUnjointObject&) override;
    void visit(const OpGraphPatternRoot&) override;

    QuadModel& model;
    std::vector<query::ast::SelectItem> select_items;
    std::map<std::string, VarId> var_name2var_id;
    std::set<VarId> assigned_vars;
    std::unique_ptr<BindingIdIter> tmp;
    // int_fast32_t id_count = 0;

    VarId get_var_id(const std::string& var_name);
    ObjectId get_value_id(const common::ast::Value& value);

    std::unique_ptr<BindingIdIter> get_greedy_join_plan(
        std::vector<std::unique_ptr<JoinPlan>> base_plans,
        std::size_t binding_size);

};

#endif // RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_
