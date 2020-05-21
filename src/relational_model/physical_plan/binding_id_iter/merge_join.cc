#include "merge_join.h"

using namespace std;

MergeJoin::MergeJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
                     vector<VarId> join_vars) :
    lhs(move(lhs)), rhs(move(rhs)), join_vars(move(join_vars)) { }


void MergeJoin::begin(BindingId& input){
    // TODO:
}


void MergeJoin::reset(BindingId& input) {
    // TODO:
}


BindingId* MergeJoin::next(){
    // TODO:
    return nullptr;
}


void MergeJoin::construct_binding(BindingId& lhs, BindingId& rhs) {
    // TODO:
}