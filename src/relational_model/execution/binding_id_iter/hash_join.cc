#include "hash_join.h"

#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

HashJoin::HashJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
        std::vector<VarId> left_vars, std::vector<VarId> common_vars, std::vector<VarId> right_vars) :
    lhs          (move(lhs)),
    rhs          (move(rhs)),
    left_vars    (left_vars),
    common_vars  (common_vars),
    right_vars   (right_vars),
    lhs_hash     (MultiMap(common_vars.size(), left_vars.size())), // empty initialization
    rhs_hash     (MultiMap(common_vars.size(), right_vars.size()))
    { }


void HashJoin::begin(BindingId& _parent_binding, bool /*parent_has_next*/) {
    this->parent_binding = &_parent_binding;
    // TODO: if we want hash join to work together with index nested loop join we need to fix this
    lhs->begin(_parent_binding, true);
    rhs->begin(_parent_binding, true);

    cout << "VARS: " << endl;
    for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
        cout << left_vars[i].id << "-";
    }
    cout << endl;
    for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
        cout << common_vars[i].id << "-";
    }
    cout << endl;
    for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
        cout << right_vars[i].id << "-";
    }
    cout << endl;

    current_key = std::vector<ObjectId>(common_vars.size());
    current_value = std::vector<ObjectId>(left_vars.size());
    while (lhs->next()){
        // save left keys and value
        for (size_t i = 0; i < common_vars.size(); i++) {
            current_key[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < left_vars.size(); i++) {
            current_value[i] = (*parent_binding)[left_vars[i]];
        }
        // TODO: cout current_key, and current value
        cout << "Key left: ";
        for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
            cout << current_key[i].id << "-";
        }
        cout << endl;

        cout << "Value left: ";
        for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
            cout << current_value[i].id << "-";
        }
        cout << endl;

        lhs_hash.insert(current_key, current_value);
    }
    current_value = std::vector<ObjectId>(right_vars.size());
    while (rhs->next()) {
        for (size_t i = 0; i < common_vars.size(); i++) {
            current_key[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < right_vars.size(); i++) {
            current_value[i] = (*parent_binding)[right_vars[i]];
        }
        // TODO: cout current_key, and current value
        cout << "Key right: ";
        for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
            cout << current_key[i].id << "-";
        }
        cout << endl;

        cout << "Value right: ";
        for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
            cout << current_value[i].id << "-";
        }
        cout << endl;
        rhs_hash.insert(current_key, current_value);
    }
    current_bucket = 0;
    current_pos_left = 0;
    current_pos_right = 0;
}


// last_k_bits(hash(key)) => decide bucket entre 0 y 2^k -1
bool HashJoin::next() {
    // TODO: current next doesnt use hash in memory
    while (current_bucket < rhs_hash.MAX_BUCKETS) { // TODO: por ahora MAX_BUCKETS esta hardcodeado
        // cout << "next: "<< current_bucket << endl;
        // cout << current_pos_left << endl;
        // cout << current_pos_right << endl;
        // cout << rhs_hash.bucket_size(current_bucket) << endl;
        // cout << lhs_hash.bucket_size(current_bucket) << endl;
        if (current_pos_left < lhs_hash.bucket_size(current_bucket)) {
            while (current_pos_right < rhs_hash.bucket_size(current_bucket)) {
                auto left_pair = lhs_hash.get_pair(current_bucket, current_pos_left);  // reference
                auto right_pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                if (left_pair.first == right_pair.first) {
                    for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
                        parent_binding->add(left_vars[i], left_pair.second[i]);
                    }
                    for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
                        parent_binding->add(common_vars[i], left_pair.first[i]);
                    }
                    for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
                        parent_binding->add(right_vars[i], right_pair.second[i]);
                    }
                    current_pos_right++;
                    return true;
                } else {
                    current_pos_right++;
                }
            }
            current_pos_left++;
            current_pos_right = 0;
        }
        else {
            current_pos_left = 0;
            current_bucket++;
        }
    }
    return false;
}


void HashJoin::reset() {
    // TODO:
}


void HashJoin::assign_nulls() {
    // rhs->assign_nulls();
    // lhs->assign_nulls();
}


void HashJoin::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
         cout << ' ';
    }
    cout << "HashJoin(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}
