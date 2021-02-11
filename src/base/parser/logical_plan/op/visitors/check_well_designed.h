/**
 * Will throw an exception if query is not well designed
 */
#ifndef BASE__CHECK_WELL_DESIGNED_H_
#define BASE__CHECK_WELL_DESIGNED_H_

#include <set>

#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class CheckWellDesigned : public OpVisitor {
private:
    std::set<std::string> parent;
    std::set<std::string> global;

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
};

#endif // BASE__CHECK_WELL_DESIGNED_H_
