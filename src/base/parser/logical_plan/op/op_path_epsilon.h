#ifndef BASE__OP_PATH_EPSILON_H_
#define BASE__OP_PATH_EPSILON_H_

#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathEpsilon : public OpPath {
public:

    OpPathEpsilon() = default;

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpPath& other) const override {
        // TODO:
        // if (min < min) {
        //     return true;
        // } else if (max < max) {
        //     return true;
        // } else if (*op_path < *other.op_path) {
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
        os << "OpPathEpsilon()\n";

        return os;
    };

    bool nullable() const {
        return true;
    }

    std::unique_ptr<OpPath> duplicate() const override {
        return std::make_unique<OpPathEpsilon>();
    }

    OpPathType type() const { return OpPathType::OP_PATH_EPSILON; }

    PathAutomaton get_automaton() const override {
        auto automaton = PathAutomaton();
        automaton.connect_states(automaton.start, automaton.end, "");
        return automaton;
    }

};

#endif // BASE__OP_PATH_EPSILON_H_
