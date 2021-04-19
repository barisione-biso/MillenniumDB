#ifndef BASE__OP_VISITOR_H_
#define BASE__OP_VISITOR_H_

class OpSelect;
class OpMatch;
class OpFilter;
class OpConnection;
class OpConnectionType;
class OpLabel;
class OpOptional;
class OpProperty;
class OpTransitiveClosure;
class OpOrderBy;
class OpGroupBy;
class OpUnjointObject;
class OpGraphPatternRoot;
class OpDistinct;

class OpVisitor {
public:
    virtual ~OpVisitor() = default;
    virtual void visit(OpSelect&) = 0;
    virtual void visit(OpMatch&) = 0;
    virtual void visit(OpFilter&) = 0;
    virtual void visit(OpConnection&) = 0;
    virtual void visit(OpConnectionType&) = 0;
    virtual void visit(OpLabel&) = 0;
    virtual void visit(OpOptional&) = 0;
    virtual void visit(OpProperty&) = 0;
    virtual void visit(OpTransitiveClosure&) = 0;
    virtual void visit(OpGraphPatternRoot&) = 0;
    virtual void visit(OpOrderBy&) = 0;
    virtual void visit(OpGroupBy&) = 0;
    virtual void visit(OpUnjointObject&) = 0;
    virtual void visit(OpDistinct&) = 0;
};

#endif // BASE__OP_VISITOR_H_
