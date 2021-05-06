#include "leapfrog_iter.h"

#include <cassert>
#include <iostream>

#include "storage/buffer_manager.h"

template class LeapfrogIterImpl<2>;
template class LeapfrogIterImpl<3>;
template class LeapfrogIterImpl<4>;

using namespace std;

template <size_t N>
LeapfrogIterImpl<N>::LeapfrogIterImpl(const BPlusTree<N>& btree,
                                      const std::vector<ObjectId> terms,
                                      const std::vector<VarId> intersection_vars,
                                      const std::vector<VarId> enumeration_vars) :
    // btree             (btree),
    terms             (move(terms)),
    intersection_vars (move(intersection_vars)),
    enumeration_vars  (move(enumeration_vars)),
    level             (-1)
{
    auto root = btree.get_root();
    directory_stack.push( move(root) );

    // there is a border case when nothing id done, but enumeration, so we must
    // position at the first record
    array<uint64_t, N> min;
    for (size_t i = 0; i < N; i++) {
        min[i] = 0;
    }
    auto leaf_and_pos = directory_stack.top()->search_leaf(directory_stack, min);
    current_leaf = move(leaf_and_pos.leaf);
    assert(current_leaf != nullptr);
    current_pos_in_leaf = leaf_and_pos.result_index;

    // check border case when B+tree is empty
    if (current_pos_in_leaf < current_leaf->get_value_count()) {
        current_tuple = current_leaf->get_record(current_pos_in_leaf);
    } else {
        array<uint64_t, N> max;
        for (size_t i = 0; i < N; i++) {
            min[i] = UINT64_MAX;
        }
        current_tuple = make_unique<Record<N>>(max);
    }
}

/*
 * No es simplemente aumentar el level, hay que asegurarse de que se este en el primer record del level
 * y para esto es necesaria una busqueda en el B+tree
 */
template <size_t N>
void LeapfrogIterImpl<N>::down() {
    assert(current_tuple != nullptr);
    level++;

    array<uint64_t, N> min;
    array<uint64_t, N> max;

    // before the level min and max must be equal to the current_record
    for (int_fast32_t i = 0; i < level; i++) {
        min[i] = (*current_tuple)[i];
        max[i] = (*current_tuple)[i];
    }

    // from level until the end is an open range [0, MAX]
    for (uint_fast32_t i = level; i < N; i++) {
        min[i] = 0;
        max[i] = UINT64_MAX;
    }

    // TODO: estoy suponiendo que simepre encuentro algo, pero que pasa con los B+trees vacios
    internal_search(Record<N>(min), Record<N>(max));
}


template <size_t N>
bool LeapfrogIterImpl<N>::next() {
    assert(current_tuple != nullptr);
    array<uint64_t, N> min;
    array<uint64_t, N> max;

    // before level min and max are equal to the current_record
    for (int_fast32_t i = 0; i < level; i++) {
        min[i] = (*current_tuple)[i];
        max[i] = (*current_tuple)[i];
    }

    // at the same level min is 1 grater than the current record and max is unbound
    min[level] = (*current_tuple)[level] + 1;
    max[level] = UINT64_MAX;

    // after level min is 0 and max is unbound
    for (size_t i = level+1; i < N; i++) {
        min[i] = 0;
        max[i] = UINT64_MAX;
    }

    return internal_search(Record<N>(min), Record<N>(max));
}


template <size_t N>
bool LeapfrogIterImpl<N>::seek(uint64_t key) {
    assert(current_tuple != nullptr);
    array<uint64_t, N> min;
    array<uint64_t, N> max;

    // before level min and max are equal to the current_record
    for (int_fast32_t i = 0; i < level; i++) {
        min[i] = (*current_tuple)[i];
        max[i] = (*current_tuple)[i];
    }

    min[level] = (key > (*current_tuple)[level])
                ? key
                : ((*current_tuple)[level] + 1); // if the given key is already greater we still need to go forward
    max[level] = UINT64_MAX;

    // after level min is 0 and max is unbound
    for (uint_fast32_t i = level+1; i < N; i++) {
        min[i] = 0;
        max[i] = UINT64_MAX;
    }
    return internal_search(Record<N>(min), Record<N>(max));
}


// updates current_leaf, current_tuple and current_pos_in_leaf only when returns true;
template <std::size_t N>
bool LeapfrogIterImpl<N>::internal_search(const Record<N>& min, const Record<N>& max) {
    assert(current_leaf != nullptr);
    // if leaf.min <= min <= leaf.max, search inside the leaf and return
    // if (current_leaf != nullptr && current_leaf->check_range(min)) {
    if (current_leaf->check_range(min)) {
        auto new_current_pos_in_leaf = current_leaf->search_index(min);
        // check new_current_pos_in_leaf is a valid position
        if (new_current_pos_in_leaf < current_leaf->get_value_count()) {
            auto new_current_tuple = current_leaf->get_record(new_current_pos_in_leaf);
            if ((*new_current_tuple) <= max) {
                // current_leaf stays the same
                current_tuple       = move(new_current_tuple);
                current_pos_in_leaf = new_current_pos_in_leaf;
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        // else search in the stack for a dir where dir.min <= min <= dir.max
        // a dir may not have records (i.e when having one leaf as child), in that case it will return the a record with zeros
        // and conditions will be false, so its ok
        // if we don't find it we stay with the root (lowest item in the stack)
        while (directory_stack.size() > 1
               && !directory_stack.top()->check_range(min))
        {
            directory_stack.pop();
        }

        // then search until reaching the leaf_number and index of the record.
        auto leaf_and_pos = directory_stack.top()->search_leaf(directory_stack, min);
        auto new_current_leaf = move(leaf_and_pos.leaf);
        auto new_current_pos_in_leaf = leaf_and_pos.result_index;

        // check new_current_pos_in_leaf is a valid position
        // we may need to go to the first record of the next leaf
        if (new_current_pos_in_leaf >= new_current_leaf->get_value_count()) {
            if (new_current_leaf->has_next()) {
                new_current_leaf = new_current_leaf->get_next_leaf();
                new_current_pos_in_leaf = 0;
            } else {
                return false;
            }
        }

        auto new_current_tuple = new_current_leaf->get_record(new_current_pos_in_leaf);
        if ((*new_current_tuple) <= max) {
            current_tuple       = move(new_current_tuple);
            current_leaf        = move(new_current_leaf);
            current_pos_in_leaf = new_current_pos_in_leaf;
            return true;
        } else {
            return false;
        }
    }
}


template <size_t N>
void LeapfrogIterImpl<N>::enum_no_intersection(TupleBuffer& buffer) {
    assert(current_leaf != nullptr);
    assert(current_tuple != nullptr);
    buffer.reset();
    array<uint64_t, N> max;

    for (int_fast32_t i = 0; i <= level; i++) {
        max[i] = (*current_tuple)[i];
    }
    for (size_t i = level+1; i < N; i++) {
        max[i] = UINT64_MAX;
    }

    // BptIter it(SearchLeafResult<N>(move(current_leaf), current_pos_in_leaf), Record<N>(max)); // TODO: al mover current_leaf lka 2da vez muere
    BptIter it = BptIter(SearchLeafResult<N>(current_leaf->duplicate(), current_pos_in_leaf),
                         Record<N>(max));
    auto record = it.next();
    while (record != nullptr) {
        vector<ObjectId> tuple;
        for (size_t i = 0; i < enumeration_vars.size(); i++) {
            tuple.push_back( ObjectId(record->ids[terms.size() + intersection_vars.size() + i]) );
        }
        buffer.append_tuple(tuple);
        record = it.next();
    }
    // TODO: update current_tuple and current_leaf? no se si se puede porque me paso en 1 con el while anterior
    // y si justo quedo en una pagina nueva no tengo puntero para ir hacia atras
    // cout << "Intersection tuple count: " << buffer.get_tuple_count() << "\n";
}


template <size_t N>
bool LeapfrogIterImpl<N>::open_terms() {
    for (const auto& term : terms) {
        down();
        // There exists the possibility that after down()
        // it ended up finding the term. Calling seek() would move forward
        if (get_key() != term.id && !seek(term.id)) {
            return false;
        } else {
            if (get_key() != term.id) {
                return false;
            }
        }
    }
    return true;
}
