#ifndef BASE__OP_PATH_SUFFIX_H_
#define BASE__OP_PATH_SUFFIX_H_

#include <iostream>

class OpPathSuffix : public OpPath {
public:
    static constexpr uint32_t MAX = -1;
    std::unique_ptr<OpPath> op_path;
    uint32_t min;
    uint32_t max;

    OpPathSuffix(std::unique_ptr<OpPath> op_path, uint32_t min, uint32_t max) :
        op_path (std::move(op_path)),
        min     (min),
        max     (max)
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
        os << "OpSuffix(min: " << min << ",max: " << max << ")\n";
        op_path->print_to_ostream(os, indent + 2);

        return os;
    };

};

#endif // BASE__OP_PATH_SUFFIX_H_