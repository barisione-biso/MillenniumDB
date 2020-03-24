#ifndef BASE__EMPTY_MATCH_VISITOR__H_
#define BASE__EMPTY_MATCH_VISITOR__H_

#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class EmptyMatchVisitor : public OpVisitor {
public:
    void visit (OpSelect&) override;
    void visit (OpMatch&) override;
    void visit (OpFilter&) override;
    void visit (OpLabel&) override;
    void visit (OpProperty&) override;
    void visit (OpConnection&) override;
private:
    bool select_all; // true if query is like SELECT * ...
};

#endif // BASE__EMPTY_MATCH_VISITOR__H_
