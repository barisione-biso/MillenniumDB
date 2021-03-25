#ifndef BASE__OP_PATH_SEQUENCE_H_
#define BASE__OP_PATH_SEQUENCE_H_

#include <memory>
#include <vector>

#include "base/parser/logical_plan/op/op.h"

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

};

#endif // BASE__OP_PATH_SEQUENCE_H_
