#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "execution/binding_id_iter/paths/path_index.h"

namespace Paths {
/*
Provides indexes for RdfModel.
*/
class RdfModelIndexProvider : public PathIndexProvider {
private:
    // Store info about assigned indexes for each transition
    // std::unordered_map<uint64_t, IndexType> t_info;
    // std::unordered_map<uint64_t, IndexType> t_inv_info;

    // Interruption
    bool* interruption_requested;

    // Special Case: B+Tree index is stored on disk
    std::unique_ptr<PathIndexIter> get_btree_iterator(uint64_t type_id, bool inverse, uint64_t node_id);

public:
    RdfModelIndexProvider(// std::unordered_map<uint64_t, IndexType> t_info,
                          // std::unordered_map<uint64_t, IndexType> t_inv_info,
                          bool*                                   interruption_requested);

    bool                               node_exists(uint64_t node_id) override;
    std::unique_ptr<PathIndexIter> get_iterator(uint64_t predicate_id, bool inverse, uint64_t node_id) override;
};
} // namespace Paths