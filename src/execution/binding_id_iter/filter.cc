#include "filter.h"

void Filter::begin(BindingId& _parent_binding) {
    parent_binding = &_parent_binding;
    child->begin(_parent_binding);
}


bool Filter::next() {
    while (child->next()) {
        bool pass_filters = true;
        for (auto& filter : filters) {
            auto evaluation = filter->eval(*parent_binding);
            if (evaluation.id != (ObjectId::MASK_BOOL | 1UL)) { // TODO: write more elegant
                pass_filters = false;
                break;
            }
        }
        if (pass_filters) {
            passed_results++;
            return true;
        } else {
            filtered_results++;
        }
    }
    return false;
}


void Filter::reset() {
    child->reset();
}


void Filter::assign_nulls() {
    child->assign_nulls();
}


void Filter::analyze(std::ostream& os, int indent) const {
    os << std::string(indent, ' ');
    os << "Filter (filtered_results: " << filtered_results << ", passed_results: " << passed_results << " )\n";
    child->analyze(os, indent+2);
}
