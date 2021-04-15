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

    virtual std::string to_string() const = 0;
    virtual bool nullable() const = 0;
    virtual std::unique_ptr<OpPath> duplicate() const = 0;
    virtual OpPathType type() const = 0;
    // TODO: Alternatives: Pasan recursivamente. Atom: !inverse, Sequence: Da vuelta
    // la secuencia y lo pasa recursivo. Kleen Start y Optional lo pasan recursivo
    virtual std::unique_ptr<OpPath> invert() const = 0;

    bool operator<(const OpPath& other) const {
        return to_string() < other.to_string();
    }

    // TODO: Make protected
    virtual PathAutomaton get_automaton() const = 0;
    PathAutomaton get_optimized_automaton() const {
        auto automaton = get_automaton();
        automaton.optimize_automata();
        return automaton;
    }
};

#endif // BASE__OP_PATH_H_