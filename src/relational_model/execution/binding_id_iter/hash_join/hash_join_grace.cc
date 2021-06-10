#include "hash_join_grace.h"

#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

HashJoinGrace::HashJoinGrace(unique_ptr<BindingIdIter> lhs,
                   unique_ptr<BindingIdIter> rhs,
                   vector<VarId> left_vars,
                   vector<VarId> common_vars,
                   vector<VarId> right_vars) :
    lhs         (move(lhs)),
    rhs         (move(rhs)),
    left_vars   (left_vars),
    common_vars (common_vars),
    right_vars  (right_vars),
    lhs_hash    (KeyValueHash<ObjectId, ObjectId>(common_vars.size(), left_vars.size())),
    rhs_hash    (KeyValueHash<ObjectId, ObjectId>(common_vars.size(), right_vars.size()))
    { }


void HashJoinGrace::begin(BindingId& _parent_binding, bool parent_has_next) {
    this->parent_binding = &_parent_binding;

    lhs->begin(_parent_binding, parent_has_next);
    rhs->begin(_parent_binding, parent_has_next);

    saved_pair = make_pair(vector<ObjectId>(common_vars.size()), vector<ObjectId>(left_vars.size()));
    lhs_hash.begin();
    while (lhs->next()){
        // save left keys and value
        for (size_t i = 0; i < common_vars.size(); i++) {
            saved_pair.first[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < left_vars.size(); i++) {
            saved_pair.second[i] = (*parent_binding)[left_vars[i]];
        }
        lhs_hash.insert(saved_pair.first, saved_pair.second);
    }
    // recycle current_value for saving right hash
    saved_pair.second.resize(right_vars.size());
    auto left_depth = lhs_hash.get_depth();
    rhs_hash.begin(left_depth);
    while (rhs->next()) {
        for (size_t i = 0; i < common_vars.size(); i++) {
            saved_pair.first[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < right_vars.size(); i++) {
            saved_pair.second[i] = (*parent_binding)[right_vars[i]];
        }
        rhs_hash.insert(saved_pair.first, saved_pair.second);
    }
    auto right_depth = rhs_hash.get_depth();
    // split if different size
    while (right_depth > left_depth) {
        lhs_hash.split();
        left_depth = lhs_hash.get_depth();
    }
    // sort buckets for faster comparison, we assume the insertion process is over
    // TODO: it would be faster to sort only the smaller bucket for each bucket position
    lhs_hash.sort_buckets();
    rhs_hash.sort_buckets();

    current_bucket = 0;
    current_state = State::NOT_ENUM;
    left_min = false;
    total_buckets = (1 << lhs_hash.get_depth());  // rhs_hash and lhs_hash should have the same depth
}


bool HashJoinGrace::next() {
    // tengo que ver en que estado estoy:
    // 1) estoy enumerando con iterador
    // 2) ya estoy enumerando resultados de un bucket con el 2do hash
    // 3) ya estoy enumerando resultados de un bucket con nested loop
    // 4) aún no empiezo a enumerar resultados
    while (true) {
        switch (current_state)
        {
            case State::ENUM_WITH_ITER: {
                assert(current_pair_iter != end_range_iter);
                if (left_min) { // solo asignar valor de iter (left)
                    assign_left_binding((*current_pair_iter).second);
                }
                else { // solo asignar valor de iter (right)
                    assign_right_binding((*current_pair_iter).second);
                }
                ++current_pair_iter;
                if (current_pair_iter == end_range_iter) {
                    current_state = State::ENUM_WITH_SECOND_HASH;
                }
                return true;
            }
            case State::ENUM_WITH_SECOND_HASH: {
                if (left_min) {
                    // Sets the iterator for left hash (small one)
                    while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                        saved_pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                        // TODO: ask for this
                        //auto key_ptr = rhs_hash.get_key(current_bucket, current_pos_right);
                        //saved_pair.first = vector<ObjectId>(key_ptr, key_ptr + common_vars.size());
                        current_pos_right++;  // after get pair and before posible return
                        auto range = small_hash.equal_range(saved_pair.first);
                        current_pair_iter = range.first;
                        end_range_iter    = range.second;

                        if (current_pair_iter != end_range_iter) {
                            current_state = State::ENUM_WITH_ITER;
                            //assign right pair, saved key
                            //auto value_ptr = rhs_hash.get_value(current_bucket, current_pos_right - 1);
                            assign_right_binding(saved_pair.second);
                            assign_key_binding(saved_pair.first);
                            break;
                        }
                    }
                }
                else {
                    // Sets the iterator for right hash (small one)
                    while (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                        saved_pair = lhs_hash.get_pair(current_bucket, current_pos_left);
                        current_pos_left++;  // after get pair and before posible return
                        auto range = small_hash.equal_range(saved_pair.first);
                        current_pair_iter = range.first;
                        end_range_iter    = range.second;

                        if (current_pair_iter != end_range_iter) {
                            current_state = State::ENUM_WITH_ITER;
                            // assign left pair, saved key
                            assign_left_binding(saved_pair.second);
                            assign_key_binding(saved_pair.first);
                            break;
                        }
                    }
                }
                if (current_state == State::ENUM_WITH_SECOND_HASH) {
                    // if reaches here, there is no match in the current bucket
                    current_state = State::NOT_ENUM;
                    current_bucket++;
                }
                break;
            }
            case State::ENUM_WITH_NESTED_LOOP: {
                if (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                    //auto left_pair = lhs_hash.get_pair(current_bucket, current_pos_left);
                    auto left_key = lhs_hash.get_key(current_bucket, current_pos_left);
                    auto left_assigned = false;
                    while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                        //auto right_pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                        auto right_key = rhs_hash.get_key(current_bucket, current_pos_right);
                        auto match = true;
                        for (uint_fast32_t i=0; i<common_vars.size(); i++) {
                            if (left_key[i] != right_key[i]) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            auto left_value = lhs_hash.get_value(current_bucket, current_pos_left);
                            auto right_value = rhs_hash.get_value(current_bucket, current_pos_right);
                            if (!left_assigned) {
                                for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
                                    parent_binding->add(left_vars[i], left_value[i]);
                                }
                                for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
                                    parent_binding->add(common_vars[i], left_key[i]);
                                }
                                left_assigned = true;
                            }
                            for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
                                parent_binding->add(right_vars[i], right_value[i]);
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
                    current_bucket++;
                    current_state = State::NOT_ENUM;
                }
                break;
            }
            case State::NOT_ENUM: {
                if (current_bucket < total_buckets) {
                    auto left_size  = lhs_hash.get_bucket_size(current_bucket) * (left_vars.size() + common_vars.size()) * sizeof(ObjectId);
                    auto right_size = rhs_hash.get_bucket_size(current_bucket) * (right_vars.size() + common_vars.size()) * sizeof(ObjectId);

                    left_min = left_size < right_size;
                    current_pos_left = 0;
                    current_pos_right = 0;
                    if (left_min) {
                        if (left_size < MAX_SIZE_SMALL_HASH) {
                            // Add lhs results to small hash
                            small_hash.clear();
                            while (current_pos_left < lhs_hash.get_bucket_size(current_bucket)) {
                                auto pair = lhs_hash.get_pair(current_bucket, current_pos_left);
                                small_hash.insert(pair);
                                current_pos_left++;
                            }
                            current_state = State::ENUM_WITH_SECOND_HASH;
                        }
                        else {
                            current_state = State::ENUM_WITH_NESTED_LOOP;
                        }
                    }
                    else {
                        if (right_size < MAX_SIZE_SMALL_HASH) {
                            // Add lhs results to small hash
                            small_hash.clear();
                            while (current_pos_right < rhs_hash.get_bucket_size(current_bucket)) {
                                auto pair = rhs_hash.get_pair(current_bucket, current_pos_right);
                                small_hash.insert(pair);
                                current_pos_right++;
                            }
                            current_state = State::ENUM_WITH_SECOND_HASH;
                        }
                        else {
                            current_state = State::ENUM_WITH_NESTED_LOOP;
                        }
                    }
                }
                else {
                    return false;
                }
                break;
            }
        }
    }
    return false;
}


void HashJoinGrace::assign_left_binding(const vector<ObjectId>& left_value) {
    for (uint_fast32_t i = 0; i < left_vars.size(); i++) {
        parent_binding->add(left_vars[i], left_value[i]);
    }
}

void HashJoinGrace::assign_right_binding(const vector<ObjectId>& right_value) {
    for (uint_fast32_t i = 0; i < right_vars.size(); i++) {
        parent_binding->add(right_vars[i], right_value[i]);
    }
}

void HashJoinGrace::assign_key_binding(const vector<ObjectId>& my_key) {
    for (uint_fast32_t i = 0; i < common_vars.size(); i++) {
        parent_binding->add(common_vars[i], my_key[i]);
    }
}


void HashJoinGrace::reset() {
    // analogous to begin
    lhs->reset();
    rhs->reset();

    saved_pair.second.resize(left_vars.size());
    lhs_hash.reset();
    while (lhs->next()) {
        for (size_t i = 0; i < common_vars.size(); i++) {
            saved_pair.first[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < left_vars.size(); i++) {
            saved_pair.second[i] = (*parent_binding)[left_vars[i]];
        }
        lhs_hash.insert(saved_pair.first, saved_pair.second);
    }
    auto left_depth = lhs_hash.get_depth();
    rhs_hash.reset(left_depth);
    saved_pair.second.resize(right_vars.size());
    while (rhs->next()) {
        for (size_t i = 0; i < common_vars.size(); i++) {
            saved_pair.first[i] = (*parent_binding)[common_vars[i]];
        }
        for (size_t i = 0; i < right_vars.size(); i++) {
            saved_pair.second[i] = (*parent_binding)[right_vars[i]];
        }
        rhs_hash.insert(saved_pair.first, saved_pair.second);
    }
    auto right_depth = rhs_hash.get_depth();
    // split if different size
    while (right_depth > left_depth) {
        lhs_hash.split();
        left_depth = lhs_hash.get_depth();
    }
    // sort buckets for faster comparison, we assume the insertion process is over
    // TODO: it would be faster to sort only the smaller bucket for each bucket position
    lhs_hash.sort_buckets();
    rhs_hash.sort_buckets();

    current_bucket = 0;
    current_state = State::NOT_ENUM;
    left_min = false;
    total_buckets = (1 << lhs_hash.get_depth());  // rhs_hash and lhs_hash should have the same depth
}


void HashJoinGrace::assign_nulls() {
    rhs->assign_nulls();
    lhs->assign_nulls();
}


void HashJoinGrace::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
         cout << ' ';
    }
    cout << "HashJoinGrace(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}
