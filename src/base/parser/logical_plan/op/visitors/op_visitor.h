#ifndef BASE__OP_VISITOR_H_
#define BASE__OP_VISITOR_H_

class OpSelect;
class OpMatch;
class OpFilter;
class OpNodeLabel;
class OpEdgeLabel;
class OpNodeProperty;
class OpEdgeProperty;
class OpConnection;
class OpLonelyNode;
class OpNodeLoop;

class OpVisitor {
public:
    virtual ~OpVisitor() { };
    virtual void visit(OpSelect&) = 0;
    virtual void visit(OpMatch&) = 0;
    virtual void visit(OpFilter&) = 0;
    virtual void visit(OpNodeLabel&) = 0;
    virtual void visit(OpEdgeLabel&) = 0;
    virtual void visit(OpNodeProperty&) = 0;
    virtual void visit(OpEdgeProperty&) = 0;
    virtual void visit(OpConnection&) = 0;
    virtual void visit(OpLonelyNode&) = 0;
    virtual void visit(OpNodeLoop&) = 0;
};

#endif // BASE__OP_VISITOR_H_
