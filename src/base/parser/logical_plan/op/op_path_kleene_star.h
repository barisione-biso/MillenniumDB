#ifndef BASE__OP_PATH_KLEENE_STAR_H_
#define BASE__OP_PATH_KLEENE_STAR_H_

#include <memory>
#include <vector>

#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/visitors/simplify_property_path.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathKleeneStar : public OpPath {
public:
    std::unique_ptr<OpPath> path;

    OpPathKleeneStar(std::unique_ptr<OpPath> path) :
        // path  (std::move(path))
        path  ( SimplifyPropertyPath::denull(std::move(path)) )
        { }

    OpPathKleeneStar(const OpPathKleeneStar& other) :
        path  (other.path->duplicate())
        { }

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
        auto kleene_automaton = PathAutomaton();
        auto path_automaton = path->get_automaton();
        path_automaton.connect_states(path_automaton.end, path_automaton.start, "");
        kleene_automaton.merge_with_automaton(path_automaton);
        kleene_automaton.connect_states(kleene_automaton.start, kleene_automaton.end, "");
        kleene_automaton.connect_states(kleene_automaton.start, path_automaton.start, "");
        kleene_automaton.connect_states(path_automaton.end, kleene_automaton.end, "");
        return kleene_automaton;
    }
};

#endif // BASE__OP_PATH_KLEENE_STAR_H_
