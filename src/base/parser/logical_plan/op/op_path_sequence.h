#ifndef BASE__OP_PATH_SEQUENCE_H_
#define BASE__OP_PATH_SEQUENCE_H_

#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op.h"

class OpPathSequence : public OpPath {
public:
    std::vector<std::unique_ptr<OpPath>> sequence;

    OpPathSequence(std::vector<std::unique_ptr<OpPath>> sequence) :
        sequence (std::move(sequence))
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
        os << "OpSequence()\n";

        for (auto& seq : sequence) {
            seq->print_to_ostream(os, indent + 2);
        }

        return os;
    };
};

#endif // BASE__OP_PATH_SEQUENCE_H_
