#ifndef BASE__OP_PATH_ALTERNATIVES_H_
#define BASE__OP_PATH_ALTERNATIVES_H_

#include <memory>
#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathAlternatives : public OpPath {
public:
    std::vector<std::unique_ptr<OpPath>> alternatives;
    const bool is_nullable;

    static bool get_nullable(const std::vector<std::unique_ptr<OpPath>>& alternatives) {
        for (const auto& alternative : alternatives) {
            if (alternative->nullable()) {
                return true;
            }
        }
        return false;
    }

    OpPathAlternatives(std::vector<std::unique_ptr<OpPath>> _alternatives) :
        alternatives (std::move(_alternatives)),
        is_nullable  (get_nullable(alternatives))
        { }

    OpPathAlternatives(const OpPathAlternatives& other) :
        is_nullable  (other.is_nullable)
    {
        for (const auto& alternative : other.alternatives) {
            alternatives.push_back(alternative->duplicate());
        }
    }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    //bool operator<(const OpPath& other) const override {
    //    return to_string() < other.to_string();
    //}

    std::string to_string() const override {
        std::string alternative_string = "(";
        alternative_string.append(alternatives[0]->to_string());
        for (std::size_t i = 1; i < alternatives.size(); i++) {
            alternative_string.append("|");
            alternative_string.append(alternatives[i]->to_string());
        }
        alternative_string.append(")");
        return alternative_string;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpAlternatives()\n";

        for (auto& alternative : alternatives) {
            alternative->print_to_ostream(os, indent + 2);
        }

        return os;
    };

    bool nullable() const {
        return is_nullable;
    }

    std::unique_ptr<OpPath> duplicate() const override {
        return std::make_unique<OpPathAlternatives>(*this);
    }

    OpPathType type() const { return OpPathType::OP_PATH_ALTERNATIVES; }

    PathAutomaton get_automaton() const override {
        auto alternative_automaton = PathAutomaton();
        for (auto& alternative : alternatives) {
            auto automaton = alternative->get_automaton();
            alternative_automaton.rename_and_merge(automaton);
            alternative_automaton.add_epsilon_transition(alternative_automaton.start, automaton.start);
            for(auto& end_state : automaton.end) {
                alternative_automaton.end.insert(end_state);
            }
        }
        return alternative_automaton;
    }
};

#endif // BASE__OP_PATH_ALTERNATIVES_H_
