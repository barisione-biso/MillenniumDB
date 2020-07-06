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
    std::set<std::string> node_names;
    std::set<std::string> edge_names;

public:
    void visit(OpSelect&) override;
    void visit(OpMatch&) override;
    void visit(OpFilter&) override;
    void visit(OpNodeLabel&) override;
    void visit(OpEdgeLabel&) override;
    void visit(OpNodeProperty&) override;
    void visit(OpEdgeProperty&) override;
    void visit(OpConnection&) override;
    void visit(OpLonelyNode&) override;
    void visit(OpNodeLoop&) override;
};

#endif // BASE__CHECK_VAR_NAMES_EXISTS_H_
