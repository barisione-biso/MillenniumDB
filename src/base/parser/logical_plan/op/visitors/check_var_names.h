/**
 * Will throw an exception if unexisting nodes/edges are used inside SELECT or WHERE
 */
#ifndef BASE__CHECK_VAR_NAMES_EXISTS_H_
#define BASE__CHECK_VAR_NAMES_EXISTS_H_

#include <set>
#include <string>

#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class CheckVarNames : public OpVisitor {
private:
    std::set<std::string> declared_object_names;

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
    void visit(OpPredicatePath&) override;
    void visit(OpInversePath&) override;
    void visit(OpSequencePath&) override;
    void visit(OpAlternativePath&) override;
};

#endif // BASE__CHECK_VAR_NAMES_EXISTS_H_
