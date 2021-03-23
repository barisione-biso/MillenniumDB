#ifndef BASE__OP_PATH_ALTERNATIVES_H_
#define BASE__OP_PATH_ALTERNATIVES_H_

#include <string>


#include "base/parser/logical_plan/op/op_path.h"

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

    OpPathAlternatives(OpPathAlternatives& path_alternative) :
        is_nullable  (path_alternative.is_nullable)
        {
            for (auto& alternative : path_alternative.alternatives) {
                alternatives.push_back(alternative->duplicate());
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
        os << "OpAlternatives()\n";

        for (auto& alternative : alternatives) {
            alternative->print_to_ostream(os, indent + 2);
        }

        return os;
    };

    bool nullable() const {
        return is_nullable;
    }

    std::unique_ptr<OpPath> duplicate() override {
        return std::make_unique<OpPathAlternatives>(*this);
    }
};

#endif // BASE__OP_PATH_ALTERNATIVES_H_
