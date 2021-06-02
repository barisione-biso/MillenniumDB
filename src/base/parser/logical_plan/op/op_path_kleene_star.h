#ifndef BASE__OP_PATH_KLEENE_STAR_H_
#define BASE__OP_PATH_KLEENE_STAR_H_

#include <memory>
#include <vector>
#include <tuple>

#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/property_paths/path_denull.h"
#include "base/parser/logical_plan/op/property_paths/path_automaton.h"

class OpPathKleeneStar : public OpPath {
public:
    std::unique_ptr<OpPath> path;

    OpPathKleeneStar(std::unique_ptr<OpPath> path) :
        //path  (std::move(path))
        path  ( PathDenull::denull(std::move(path)) )
        { }

    OpPathKleeneStar(const OpPathKleeneStar& other) :
        path  (other.path->duplicate())
        { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::string to_string() const override {
        std::string res = "(";
        res.append(path->to_string());
        res.append(")*");
        return res;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpPathKleeneStar()\n";
        path->print_to_ostream(os, indent +2);

        return os;
    };

    bool nullable() const {
        return true;
    }

    std::unique_ptr<OpPath> duplicate() const override {
        return std::make_unique<OpPathKleeneStar>(*this);
    }

    OpPathType type() const { return OpPathType::OP_PATH_KLEENE_STAR; }

    PathAutomaton get_automaton() const override {
        auto path_automaton = path->get_automaton();
        // Heuristic for kleene star consutriction
        if (path_automaton.total_states == 2) {
            // Automaton with 2 states have only one connection from 0 to 1
            auto new_automaton = PathAutomaton();
            auto& transition = path_automaton.from_to_connections[0][0];
            new_automaton.connect(Transition(0,0, transition.label, transition.inverse));
            new_automaton.end.insert(new_automaton.start);
            return new_automaton;
        }
        else {
            for (auto& end_state : path_automaton.end) {
                path_automaton.add_epsilon_transition(end_state, path_automaton.start);
            }
            path_automaton.add_end_state(path_automaton.start);
            return path_automaton;
        }
    }

    std::unique_ptr<OpPath> invert() const {
        return std::make_unique<OpPathKleeneStar>(path->invert());
    }
};

#endif // BASE__OP_PATH_KLEENE_STAR_H_
