#include "hash_join.h"

#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

HashJoin::HashJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
        std::vector<VarId> left_vars, std::vector<VarId> common_vars, std::vector<VarId> right_vars) :
    lhs             (move(lhs)),
    rhs             (move(rhs)),
    left_vars       (left_vars),
    common_vars     (common_vars),
    right_vars      (right_vars),
    lhs_hash        (KeyValueHash(common_vars.size(), left_vars.size())),
    rhs_hash        (KeyValueHash(common_vars.size(), right_vars.size())),
    small_hash      (SmallMultiMap({}))  // empty initialization
    { }


void HashJoin::begin(BindingId& _parent_binding, bool parent_has_next) {
    this->parent_binding = &_parent_binding;

    lhs->begin(_parent_binding, parent_has_next);
    rhs->begin(_parent_binding, parent_has_next);

    current_key = std::vector<ObjectId>(common_vars.size());
    current_value = std::vector<ObjectId>(left_vars.size());
    lhs_hash.begin();
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
    // recycle current_value for saving right hash
    current_value = std::vector<ObjectId>(right_vars.size());
    auto left_depth = lhs_hash.get_depth();
    rhs_hash.begin(left_depth);
    while (rhs->next()) {
        for (size_t i = 0; i < common_vars.size(); i++) {
            current_key[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < right_vars.size(); i++) {
            current_value[i] = (*parent_binding)[right_vars[i]];
        }
        rhs_hash.insert(current_key, current_value);
    }
    auto right_depth = rhs_hash.get_depth();
    // split if different size
    while (right_depth > left_depth) {
        lhs_hash.split();
        left_depth = lhs_hash.get_depth();
    }

    current_bucket = 0;
    enumerating_with_nested_loop = false;
    enumerating_with_second_hash = false;
    left_min = false;
}


bool HashJoin::next() {
    // tengo que ver en que estado estoy:
    // 1) ya estoy enumerando resultados de un bucket con el 2do hash
    // 1b) ya estoy enumerando resultados de un bucket con el 2do hash y ya encontre un range
    // 2) ya estoy enumerando resultados de un bucket con nested loop
    // 3) aún no empiezo a enumerar resultados
    const uint_fast32_t total_buckets = (1 << lhs_hash.get_depth());  // rhs_hash and lhs_hash should have the same depth
    while (true) {
        if (enumerating_with_second_hash) {
            if (left_min) {
                if (current_pair_iter != end_range_iter) {
                    assign_binding(*current_pair_iter, saved_pair);
                    ++current_pair_iter;
                    return true;
                }
                else {
                    while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                        saved_pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                        current_pos_right++;  // after get pair and before posible return
                        auto range = small_hash.equal_range(saved_pair.first);
                        current_pair_iter = range.first;
                        end_range_iter = range.second;

                        if (current_pair_iter != end_range_iter) {
                            assign_binding(*current_pair_iter, saved_pair);
                            ++current_pair_iter;
                            return true;
                        }
                    }
                }
            }
            else {
                if (current_pair_iter != end_range_iter) {
                    assign_binding(saved_pair, *current_pair_iter);
                    ++current_pair_iter;
                    return true;
                }
                else {
                    while (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                        saved_pair = lhs_hash.get_pair(current_bucket, current_pos_left);
                        current_pos_left++;  // after get pair and before posible return
                        auto range = small_hash.equal_range(saved_pair.first);
                        current_pair_iter = range.first;
                        end_range_iter = range.second;

                        if (current_pair_iter != end_range_iter) {
                            assign_binding(saved_pair, *current_pair_iter);
                            ++current_pair_iter;
                            return true;
                        }
                    }
                }
            }
            enumerating_with_second_hash = false;
            current_bucket++;
        }
        else if (enumerating_with_nested_loop) {
            if (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                    auto left_pair = lhs_hash.get_pair(current_bucket, current_pos_left);  // reference
                    auto right_pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                    if (left_pair.first == right_pair.first) {
                        assign_binding(left_pair, right_pair);
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
                current_bucket++;
                enumerating_with_nested_loop = false;
            }
        }
        else {
            if (current_bucket < total_buckets) {
                auto left_size  = lhs_hash.get_bucket_size(current_bucket) * (left_vars.size() + common_vars.size()) * sizeof(ObjectId);
                auto right_size = rhs_hash.get_bucket_size(current_bucket) * (right_vars.size() + common_vars.size()) * sizeof(ObjectId);

                left_min = left_size < right_size;

                if (left_min) {
                    if (left_size < MAX_SIZE_SMALL_HASH) {
                        enumerating_with_second_hash = true;
                        current_pos_right = 0;
                        current_pair_iter = small_hash.end();
                        end_range_iter = small_hash.end();

                        current_pos_left = 0;
                        while (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                            auto pair = lhs_hash.get_pair(current_bucket, current_pos_left);
                            small_hash.insert(pair);
                            current_pos_left++;
                        }

                    }
                    else {
                        enumerating_with_nested_loop = true;
                        current_pos_left = 0;
                        current_pos_right = 0;
                    }
                }
                else {
                    if (right_size < MAX_SIZE_SMALL_HASH) {
                        enumerating_with_second_hash = true;
                        current_pos_left = 0;
                        current_pair_iter = small_hash.end();
                        end_range_iter = small_hash.end();

                        current_pos_right= 0;
                        while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                            auto pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                            small_hash.insert(pair);
                            current_pos_right++;
                        }
                    }
                    else {
                        enumerating_with_nested_loop = true;
                        current_pos_left = 0;
                        current_pos_right = 0;
                    }
                }
            }
            else {
                return false;
            }
        }
    }

    return false;
}


void HashJoin::assign_binding(const KeyValuePair& left_pair, const KeyValuePair& right_pair) {
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


void HashJoin::reset() {
    // analogous to begin
    lhs->reset();
    rhs->reset();

    current_value = std::vector<ObjectId>(left_vars.size());
    lhs_hash.reset();
    rhs_hash.reset();
    while (lhs->next()){
        for (size_t i = 0; i < common_vars.size(); i++) {
            current_key[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < left_vars.size(); i++) {
            current_value[i] = (*parent_binding)[left_vars[i]];
        }
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
        rhs_hash.insert(current_key, current_value);
    }

    current_bucket = 0;
    enumerating_with_nested_loop = false;
    enumerating_with_second_hash = false;
    left_min = false;
}


void HashJoin::assign_nulls() {
    rhs->assign_nulls();
    lhs->assign_nulls();
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
