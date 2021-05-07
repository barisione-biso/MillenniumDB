#include "hash_join_in_memory_2.h"

#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

HashJoinInMemory2::HashJoinInMemory2(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
        std::vector<VarId> left_vars, std::vector<VarId> common_vars, std::vector<VarId> right_vars) :
    lhs             (move(lhs)),
    rhs             (move(rhs)),
    left_vars       (left_vars),
    common_vars     (common_vars),
    right_vars      (right_vars),
    lhs_hash        (MultiMap(common_vars.size(), left_vars.size()))  // empty initialization
    { }


void HashJoinInMemory2::begin(BindingId& _parent_binding, bool parent_has_next) {
    this->parent_binding = &_parent_binding;

    lhs->begin(_parent_binding, parent_has_next);
    rhs->begin(_parent_binding, parent_has_next);
    lhs_hash.begin();

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
        lhs_hash.insert(current_key, current_value);
    }

    current_value.resize(right_vars.size());
    saved_pair = make_pair(current_key, current_value);  // init sizes
    enumerating = false;
}


bool HashJoinInMemory2::next() {
    while (true) {
        if (enumerating) {
            auto left_pair = lhs_hash.get_pair(current_bucket, current_bucket_pos);
            while (left_pair.first != saved_pair.first) {  // TODO: maybe we can use current key instead savedpair
                current_bucket_pos++;
                if (current_bucket_pos >= lhs_hash.get_bucket_size(current_bucket)) {
                    enumerating = false;
                    break;
                }
                left_pair = lhs_hash.get_pair(current_bucket, current_bucket_pos);
            }
            if (!enumerating) {
                continue;
            }
            assign_binding(left_pair, saved_pair);
            current_bucket_pos++;
            if (current_bucket_pos >= lhs_hash.get_bucket_size(current_bucket)) {
                enumerating = false;
            }
            return true;
        }
        else {
            if (rhs->next()) {
                for (size_t i = 0; i < common_vars.size(); i++) {
                    saved_pair.first[i] = (*parent_binding)[common_vars[i]];
                }
                for (size_t i = 0; i < right_vars.size(); i++) {
                    saved_pair.second[i] = (*parent_binding)[right_vars[i]];
                }
                current_bucket_pos = 0;
                current_bucket = lhs_hash.get_bucket(saved_pair.first);
                if (lhs_hash.get_bucket_size(current_bucket) > 0) {
                    enumerating = true;
                }
            }
            else {
                return false;
            }
        }
    }
}


void HashJoinInMemory2::assign_binding(const MultiPair& left_pair, const MultiPair& right_pair) {
    for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
        parent_binding->add(left_vars[i], left_pair.second[i]);
    }
    for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
        parent_binding->add(common_vars[i], left_pair.first[i]);
    }
    for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
        parent_binding->add(right_vars[i], right_pair.second[i]);
    }
}


void HashJoinInMemory2::reset() {
    lhs->reset();
    rhs->reset();

    current_value = std::vector<ObjectId>(left_vars.size());
    lhs_hash.reset();
    while (lhs->next()){
        // save left keys and value
        for (size_t i = 0; i < common_vars.size(); i++) {
            current_key[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < left_vars.size(); i++) {
            current_value[i] = (*parent_binding)[left_vars[i]];
        }
        lhs_hash.insert(current_key, current_value);
    }
    current_value = std::vector<ObjectId>(right_vars.size());
    enumerating = false;
}


void HashJoinInMemory2::assign_nulls() {
    rhs->assign_nulls();
    lhs->assign_nulls();
}


void HashJoinInMemory2::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
         cout << ' ';
    }
    cout << "HashJoinInMemory2(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}
