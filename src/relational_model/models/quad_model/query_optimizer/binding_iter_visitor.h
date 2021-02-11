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
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "base/ids/object_id.h"
#include "base/ids/var_id.h"

using Id = std::variant<VarId, ObjectId>;

class BindingIter;
class OpOptional;
class OpFilter;
class OpGraphPatternRoot;

// using Id = std::variant<VarId, ObjectId>;

class BindingIterVisitor : public OpVisitor {
public:
    BindingIterVisitor(QuadModel& model, std::set<std::string> var_names);
    ~BindingIterVisitor() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);
    std::unique_ptr<BindingIter> exec(manual_plan::ast::ManualRoot&);
    std::unique_ptr<ScanRange> get_scan_range(Id id, bool assigned);

    void visit(OpSelect&) override;
    void visit(OpMatch&) override;
    void visit(OpFilter&) override;
    void visit(OpConnection&) override;
    void visit(OpConnectionType&) override;
    void visit(OpLabel&) override;
    void visit(OpProperty&) override;
    void visit(OpGroupBy&) override;
    void visit(OpOrderBy&) override;
    void visit(OpOptional&) override;
    void visit(OpTransitiveClosure&) override;
    void visit(OpUnjointObject&) override;
    void visit(OpGraphPatternRoot&) override;

    QuadModel& model;
    const std::map<std::string, VarId> var_name2var_id;
    std::unique_ptr<BindingIter> tmp;
    std::vector<query::ast::SelectItem> select_items;

    VarId get_var_id(const std::string& var_name);

    static std::map<std::string, VarId> construct_var_name2var_id(std::set<std::string>& var_names);
};

#endif // RELATIONAL_MODEL__BINDING_ITER_VISITOR_H_
