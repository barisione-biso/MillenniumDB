#ifndef RELATIONAL_MODEL__BINDING_ITER_VISITOR_H_
#define RELATIONAL_MODEL__BINDING_ITER_VISITOR_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "base/parser/logical_plan/var.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"

class BindingIter;
class OpOptional;
class OpFilter;
class OpGraphPatternRoot;

class BindingIterVisitor : public OpVisitor {
public:
    const QuadModel& model;
    const std::map<Var, VarId> var2var_id;
    std::unique_ptr<BindingIter> tmp;
    std::vector<query::ast::SelectItem> select_items;
    bool distinct_into_id = false;

    BindingIterVisitor(const QuadModel& model, std::set<Var> var_names);
    ~BindingIterVisitor() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);
    std::unique_ptr<BindingIter> exec(manual_plan::ast::ManualRoot&);

    VarId get_var_id(const Var& var_name) const;
    static std::map<Var, VarId> construct_var2var_id(std::set<Var>& var_names);

    void visit(OpSelect&) override;
    void visit(OpMatch&) override;
    void visit(OpFilter&) override;
    void visit(OpConnection&) override;
    void visit(OpLabel&) override;
    void visit(OpProperty&) override;
    void visit(OpGroupBy&) override;
    void visit(OpOrderBy&) override;
    void visit(OpOptional&) override;
    void visit(OpUnjointObject&) override;
    void visit(OpGraphPatternRoot&) override;
    void visit(OpDistinct&) override;
    void visit(OpPropertyPath&) override;
    void visit(OpPath&) override;
    void visit(OpPathAtom&) override;
    void visit(OpPathAlternatives&) override;
    void visit(OpPathSequence&) override;
    void visit(OpPathKleeneStar&) override;
    void visit(OpPathOptional&) override;
};

#endif // RELATIONAL_MODEL__BINDING_ITER_VISITOR_H_
