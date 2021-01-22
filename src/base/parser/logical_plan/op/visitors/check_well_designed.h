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
    void visit(const OpSelect&) override;
    void visit(const OpMatch&) override;
    void visit(const OpFilter&) override;
    void visit(const OpConnection&) override;
    void visit(const OpConnectionType&) override;
    void visit(const OpLabel&) override;
    void visit(const OpProperty&) override;
    void visit(const OpOrderBy&) override;
    void visit(const OpGroupBy&) override;
    void visit(const OpOptional&) override;
    void visit(const OpTransitiveClosure&) override;
    void visit(const OpUnjointObject&) override;
    void visit(const OpGraphPatternRoot&) override;
};

#endif // BASE__CHECK_WELL_DESIGNED_H_
