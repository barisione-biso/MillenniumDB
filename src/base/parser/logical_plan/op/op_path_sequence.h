#ifndef BASE__OP_PATH_SEQUENCE_H_
#define BASE__OP_PATH_SEQUENCE_H_

#include <memory>
#include <vector>
#include <iostream>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"

class OpPathSequence : public OpPath {
public:
    std::vector<std::unique_ptr<OpPath>> sequence;
    const bool is_nullable;

     static bool get_nullable(const std::vector<std::unique_ptr<OpPath>>& sequence) {
        for (const auto& seq : sequence) {
            if (seq->nullable()) {
                return false;
            }
        }
        return true;
    }

    OpPathSequence(std::vector<std::unique_ptr<OpPath>> _sequence) :
        sequence (std::move(_sequence)),
        is_nullable (get_nullable(sequence))
        { }

    OpPathSequence(const OpPathSequence& other) :
        is_nullable     (other.nullable())
    {
        for (const auto& seq : other.sequence) {
            sequence.push_back(seq->duplicate());
        }
    }

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
        os << "OpSequence()\n";

        for (auto& seq : sequence) {
            seq->print_to_ostream(os, indent + 2);
        }

        return os;
    };

    bool nullable() const {
        return is_nullable;
    }

    // void denull() override {
    //     if (!nullable()) {
    //         return;
    //     }
    //     for (const auto& alternative : alternatives) {
    //         alternative->denull();
    //     }
    // }

    std::unique_ptr<OpPath> duplicate() const override {
        return std::make_unique<OpPathSequence>(*this);
    }

    OpPathType type() const { return OpPathType::OP_PATH_SEQUENCE; }

    PathAutomaton get_automaton() const override {
        auto sequence_automaton = sequence[0]->get_automaton();
        for (size_t i = 1; i < sequence.size(); i++) {
            auto seq_automaton = sequence[i]->get_automaton();
            sequence_automaton.merge_with_automaton(seq_automaton);
            for (auto& end_state : sequence_automaton.end) {
                sequence_automaton.add_epsilon_transition(end_state, seq_automaton.start);
            }
            //TODO: Chequear si se copia bien
            sequence_automaton.end = std::move(seq_automaton.end);
        }
        return sequence_automaton;
    }

};

#endif // BASE__OP_PATH_SEQUENCE_H_
