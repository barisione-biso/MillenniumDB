/**
 * Will throw an exception if unexisting nodes/edges are used inside SELECT or WHERE
 */
#ifndef BASE__CHECK_VAR_NAMES_EXISTS_H_
#define BASE__CHECK_VAR_NAMES_EXISTS_H_

#include <set>
#include <string>

#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
// #include "base/parser/logical_plan/var.h"

class CheckVarNames : public OpVisitor {
private:
    std::set<std::string> declared_object_names; // TODO: should be a set of Var?

public:
    void visit(OpDistinct&)         override;
    void visit(OpFilter&)           override;
    void visit(OpGraphPatternRoot&) override;
    void visit(OpGroupBy&)          override;
    void visit(OpMatch&)            override;
    void visit(OpOptional&)         override;
    void visit(OpOrderBy&)          override;
    void visit(OpSelect&)           override;

    void visit(OpConnection&)       override { }
    void visit(OpIsolatedTerm&)     override { }
    void visit(OpIsolatedVar&)      override { }
    void visit(OpLabel&)            override { }
    void visit(OpPath&)             override { }
    void visit(OpPathAlternatives&) override { }
    void visit(OpPathAtom&)         override { }
    void visit(OpPathSequence&)     override { }
    void visit(OpPathKleeneStar&)   override { }
    void visit(OpPathOptional&)     override { }
    void visit(OpProperty&)         override { }
    void visit(OpPropertyPath&)     override { }
};

#endif // BASE__CHECK_VAR_NAMES_EXISTS_H_
