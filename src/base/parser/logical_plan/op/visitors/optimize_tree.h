#ifndef BASE__OPTIMIZE_TREE_H_
#define BASE__OPTIMIZE_TREE_H_

#include <set>
#include <vector>

#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op.h" // NEW
#include "base/parser/logical_plan/op/visitors/op_visitor.h"


class OptimizeTree : public OpVisitor {
private:
    std::set<OpProperty> global_properties_set;
    std::set<OpLabel> global_label_set;
    std::set<std::string> global_var_names;
    // Op* parent = nullptr;
    bool delete_current = false;
    bool move_children_up = false;
    bool optional_to_match = true;
    std::vector<std::unique_ptr<Op>> optionals;

public:
    void visit(OpSelect&) override;
    void visit(OpMatch&) override;
    void visit(OpFilter&) override;
    void visit(OpConnection&) override;
    void visit(OpConnectionType&) override;
    void visit(OpLabel&) override;
    void visit(OpProperty&) override;
    void visit(OpOrderBy&) override;
    void visit(OpGroupBy&) override;
    void visit(OpOptional&) override;
    void visit(OpTransitiveClosure&) override;
    void visit(OpUnjointObject&) override;
    void visit(OpGraphPatternRoot&) override;

    void visit(OpPropertyPath&) override;
    void visit(OpPath&) override;
    void visit(OpPathAtom&) override;
    void visit(OpPathAlternatives&) override;
    void visit(OpPathSequence&) override;
    void visit(OpPathKleenStar&) override;
    void visit(OpPathEpsilon&) override;
};
#endif // BASE__OPTIMIZE_TREE_H_
