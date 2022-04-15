#pragma once

#include "parser/query/return_item/return_item.h"

class ReturnItemAgg : public ReturnItem {
public:
    std::string aggregate_func;

    Var var;

    ReturnItemAgg(const std::string& aggregate_func, Var&& var) :
        aggregate_func (aggregate_func),
        var            (std::move(var)) { }

    Var get_var() const override {
        return var;
    }

    std::set<Var> get_vars() const override {
        std::set<Var> res { var, Var(aggregate_func + '(' + var.name + ')') };
        return res;
    }

    void accept_visitor(ReturnItemVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(' ', indent) << aggregate_func << '(' << var << ')';
    }
};
