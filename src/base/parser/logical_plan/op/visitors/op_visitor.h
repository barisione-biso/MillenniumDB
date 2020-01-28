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
    virtual void visit (OpSelect&);
    virtual void visit (OpMatch&);
    virtual void visit (OpFilter&);
    virtual void visit (OpLabel&);
    virtual void visit (OpProperty&);
    virtual void visit (OpConnection&);

};

#endif //BASE__OP_VISITOR_H_
