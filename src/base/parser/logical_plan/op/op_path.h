#ifndef BASE__OP_PATH_H_
#define BASE__OP_PATH_H_

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

enum class OpPathType {
    OP_PATH_ALTERNATIVES,
    OP_PATH_SEQUENCE,
    OP_PATH_KLEENE_STAR,
    OP_PATH_ATOM,
    OP_PATH_OPTIONAL,
};

class OpPath : public Op {
public:
    virtual bool operator<(const OpPath&) const = 0;
    virtual std::string to_string() const = 0;
    virtual bool nullable() const = 0;
    virtual std::unique_ptr<OpPath> duplicate() const = 0;
    virtual OpPathType type() const = 0;
    virtual PathAutomaton get_automaton() const = 0;
};

#endif // BASE__OP_PATH_H_