#ifndef BASE__OP_PATH_ATOM_H_
#define BASE__OP_PATH_ATOM_H_

#include <memory>
#include <string>
#include <iostream>

#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathAtom : public OpPath {
public:
    std::string atom;
    bool inverse;

    OpPathAtom(std::string atom, bool inverse) :
        atom    (atom),
        inverse (inverse)
        { }

    OpPathAtom(const OpPathAtom& other) :
        atom    (other.atom),
        inverse (other.inverse)
        { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpPath& other) const override {
        // TODO:
        // if (atom < other.atom) {
        //     return true;
        // } else if (inverse < other.inverse) {
        //     return true;
        // }
        return false;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpAtom(" << atom << ",inverse:" << (inverse ? "true" : "false") << ")\n";

        return os;
    };

    bool nullable() const {
        return false;
    }

    std::unique_ptr<OpPath> duplicate() const override {
        return std::make_unique<OpPathAtom>(*this);
    }

    OpPathType type() const { return OpPathType::OP_PATH_ATOM; }

    PathAutomaton get_automaton() const override {
        auto automaton = PathAutomaton();
        automaton.connect_states(automaton.start, automaton.end, atom);
        return automaton;
    }
};

#endif // BASE__OP_PATH_ATOM_H_
