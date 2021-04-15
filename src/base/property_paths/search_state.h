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

    bool operator<(const SearchState& other) const {
        if (state < other.state) {
            return true;
        } else {
            return object_id < other.object_id;
        }
    }
};

#endif // BASE__SEARCH_STATE_H_
