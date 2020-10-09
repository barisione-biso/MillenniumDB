#include "merge_join.h"

using namespace std;

MergeJoin::MergeJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
                     VarId join_var) :
    lhs(move(lhs)), rhs(move(rhs)), join_var(join_var) { }


BindingId* MergeJoin::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());

    current_left = lhs->begin(input);
    lhs->next();

    current_right = rhs->begin(input);
    rhs->next();

    return my_binding.get();
}


void MergeJoin::reset() {
    lhs->reset();
    rhs->reset();
}


bool MergeJoin::next() {
    // TODO: remake
    // while (current_left != nullptr && current_right != nullptr) {
    //     if ( (*current_left)[join_var].id == (*current_right)[join_var].id ) {
    //         construct_binding();
    //         lhs->next();
    //         rhs->next();
    //         ++results_found;
    //         return true;
    //     } else {
    //         if ( (*current_left)[join_var].id < (*current_right)[join_var].id ) {
    //             current_left = lhs->next();
    //         } else {
    //             current_right = rhs->next();
    //         }
    //     }
    // }
    return false;
}


void MergeJoin::construct_binding() {
    my_binding->add_all(*current_left);
    my_binding->add_all(*current_right);
}


void MergeJoin::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "MergeJoin(found: " << results_found << "\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}
