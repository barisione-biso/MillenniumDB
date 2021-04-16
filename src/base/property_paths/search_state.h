#ifndef BASE__SEARCH_STATE_H_
#define BASE__SEARCH_STATE_H_

#include <cstdint>

#include "base/ids/object_id.h"

struct SearchState {
    const uint32_t state;
    const ObjectId object_id;

    SearchState(unsigned int state, ObjectId object_id) :
        state      (state),
        object_id  (object_id) { }

    //bool operator<(const SearchState& other) const {
    //    if (other.state < state) {
    //        return true;
    //    } else {
    //        return other.object_id < object_id;
    //    }
    //}

    bool operator==(const SearchState& other) const {
        return state == other.state && object_id.id == other.object_id.id;
    }
};

struct SearchStateComparator {
    bool operator() (const SearchState& lhs, const SearchState& rhs) const {
        if (lhs.state < rhs.state) {
            return true;
        } else {
            return lhs.object_id.id < rhs.object_id.id;
        }
    }
};

struct SearchStateHasher {
    std::size_t operator() (const SearchState& lhs) const {
      return lhs.state ^ lhs.object_id.id;
    }
};

#endif // BASE__SEARCH_STATE_H_
