#ifndef BASE__OP_VISITOR_H_
#define BASE__OP_VISITOR_H_

#include <string>

class OpSelect;
class OpMatch;
class OpFilter;
class OpLabel;
class OpProperty;
class OpConnection;

class OpVisitor {
public:
    virtual void visit (const OpSelect&);
    virtual void visit (const OpMatch&);
    virtual void visit (const OpFilter&);
    virtual void visit (const OpLabel&);
    virtual void visit (const OpProperty&);
    virtual void visit (const OpConnection&);

};

#endif //BASE__OP_VISITOR_H_
