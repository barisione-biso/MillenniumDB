#ifndef BASE__OP_VISITOR_H_
#define BASE__OP_VISITOR_H_

class OpSelect;
class OpMatch;
class OpFilter;
class OpConnection;
class OpConnectionType;
class OpLabel;
class OpProperty;
class OpTransitiveClosure;
class OpOrderBy;
class OpGroupBy;
class OpUnjointObject;
//class OpOrderBy;

class OpVisitor {
public:
    virtual ~OpVisitor() = default;
    virtual void visit(const OpSelect&) = 0;
    virtual void visit(const OpMatch&) = 0;
    virtual void visit(const OpFilter&) = 0;
    virtual void visit(const OpConnection&) = 0;
    virtual void visit(const OpConnectionType&) = 0;
    virtual void visit(const OpLabel&) = 0;
    virtual void visit(const OpProperty&) = 0;
    virtual void visit(const OpTransitiveClosure&) = 0;
    virtual void visit(const OpOrderBy&) = 0;
    virtual void visit(const OpGroupBy&) = 0;
    virtual void visit(const OpUnjointObject&) = 0;
};

#endif // BASE__OP_VISITOR_H_
