#ifndef BASE__OP_PATH_KLEENE_STAR_H_
#define BASE__OP_PATH_KLEENE_STAR_H_

#include <memory>
#include <vector>
#include <tuple>

#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/visitors/simplify_property_path.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathKleeneStar : public OpPath {
public:
    std::unique_ptr<OpPath> path;

    OpPathKleeneStar(std::unique_ptr<OpPath> path) :
        //path  (std::move(path))
        path  ( SimplifyPropertyPath::denull(std::move(path)) )
        { }

    OpPathKleeneStar(const OpPathKleeneStar& other) :
        path  (other.path->duplicate())
        { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpPath& other) const override {
        return to_string() < other.to_string();
    }

    std::string to_string() const override {
        return path->to_string();
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
        //auto kleene_automaton = PathAutomaton();
        auto path_automaton = path->get_automaton();
        //TODO: Chequear condicion
        if (path_automaton.total_states == 2) {
            // && path_automaton.conections.size() == 1
            // && path_automaton.conections[path_automaton.start].size() == 1 ) {
            auto new_automaton = PathAutomaton();
            auto transition = path_automaton.from_to_connections[0][0];
            new_automaton.connect(Transition(0,0, transition.label, transition.inverse));
            new_automaton.end.insert(new_automaton.start);
            return new_automaton;
        }
        else {
            for (auto& end_state : path_automaton.end) {
                path_automaton.add_epsilon_transition(end_state, path_automaton.start);
            }
            path_automaton.end.insert(path_automaton.start);
            return path_automaton;
        }
    }
};

#endif // BASE__OP_PATH_KLEENE_STAR_H_
