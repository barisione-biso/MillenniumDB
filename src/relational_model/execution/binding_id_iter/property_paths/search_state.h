#ifndef BASE__SEARCH_STATE_H_
#define BASE__SEARCH_STATE_H_

#include <cstdint>
#include <iostream>

#include "base/ids/object_id.h"

struct SearchState {
    const uint32_t state;
    const ObjectId object_id;
    const SearchState* previous;
    const bool direction;
    const ObjectId label_id;

    SearchState(unsigned int state,
                ObjectId object_id,
                const SearchState* previous,
                const bool direction,
                ObjectId label_id) :
        state      (state),
        object_id  (object_id),
        previous   (previous),
        direction  (direction),
        label_id   (label_id) { }

    ~SearchState() = default;

    bool operator<(const SearchState& other) const {
        if (state < other.state) {
            return true;
        } else if (other.state < state) {
            return false;
        } else {
            return object_id < other.object_id;
        }
    }

    bool operator==(const SearchState& other) const {
        return state == other.state && object_id.id == other.object_id.id;
    }

    SearchState duplicate() const {
        return SearchState(state, object_id, previous, direction, label_id);
    }
};


struct SearchStateHasher {
    std::size_t operator() (const SearchState& lhs) const {
      return lhs.state ^ lhs.object_id.id;
    }
};

#endif // BASE__SEARCH_STATE_H_
