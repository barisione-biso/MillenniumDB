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
    void visit(const OpSelect&) override;
    void visit(const OpMatch&) override;
    void visit(const OpFilter&) override;
    void visit(const OpConnection&) override;
    void visit(const OpConnectionType&) override;
    void visit(const OpLabel&) override;
    void visit(const OpProperty&) override;
    void visit(const OpOrderBy&) override;
    void visit(const OpGroupBy&) override;
    void visit(const OpTransitiveClosure&) override;
    void visit(const OpUnjointObject&) override;
};

#endif // BASE__CHECK_VAR_NAMES_EXISTS_H_
