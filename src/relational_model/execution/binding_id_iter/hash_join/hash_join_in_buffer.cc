#include "hash_join_in_buffer.h"

#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

HashJoinInBuffer::HashJoinInBuffer(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
        std::vector<VarId> left_vars, std::vector<VarId> common_vars, std::vector<VarId> right_vars) :
    lhs             (move(lhs)),
    rhs             (move(rhs)),
    left_vars       (left_vars),
    common_vars     (common_vars),
    right_vars      (right_vars),
    lhs_hash        (KeyValueHash<ObjectId, ObjectId>(common_vars.size(), left_vars.size()))  // empty initialization
    { }


void HashJoinInBuffer::begin(BindingId& _parent_binding, bool parent_has_next) {
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
    enumerating = false;
}


bool HashJoinInBuffer::next() {
    while (true) {
        if (enumerating) {
            bool match_found = true;
            auto left_key = lhs_hash.get_key(current_bucket, current_bucket_pos);
            for (uint_fast32_t i = 0; i < current_key.size(); i++) {
                if (current_key[i] != left_key[i]) {
                    match_found = false;
                    break;
                }
            }
            while (!match_found) { //(left_key != current_key) {
                current_bucket_pos++;
                if (current_bucket_pos >= lhs_hash.get_bucket_size(current_bucket)) {
                    enumerating = false;
                    break;
                }
                left_key = lhs_hash.get_key(current_bucket, current_bucket_pos);
                match_found = true;
                for (uint_fast32_t i = 0; i < current_key.size(); i++) {
                    if (current_key[i] != left_key[i]) {
                        match_found = false;
                        break;
                    }
                }
            }
            if (!enumerating) {
                continue;
            }
            // set lhs binding
            auto left_value = lhs_hash.get_value(current_bucket, current_bucket_pos);
            for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
                parent_binding->add(left_vars[i], left_value[i]);
            }
            current_bucket_pos++;
            if (current_bucket_pos >= lhs_hash.get_bucket_size(current_bucket)) {
                enumerating = false;
            }
            return true;
        }
        else {
            if (rhs->next()) {
                for (size_t i = 0; i < common_vars.size(); i++) {
                    current_key[i] = (*parent_binding)[common_vars[i]];
                }
                for (size_t i = 0; i < right_vars.size(); i++) {
                    current_value[i] = (*parent_binding)[right_vars[i]];
                }
                current_bucket_pos = 0;
                current_bucket = lhs_hash.get_bucket(current_key);
                if (lhs_hash.get_bucket_size(current_bucket) > 0) {
                    enumerating = true;
                    // set rhs binding
                    for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
                        parent_binding->add(common_vars[i], current_key[i]);
                    }
                    for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
                        parent_binding->add(right_vars[i], current_value[i]);
                    }
                }
            }
            else {
                return false;
            }
        }
    }
}


void HashJoinInBuffer::reset() {
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


void HashJoinInBuffer::assign_nulls() {
    rhs->assign_nulls();
    lhs->assign_nulls();
}


void HashJoinInBuffer::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
         cout << ' ';
    }
    cout << "HashJoinInBuffer(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}
