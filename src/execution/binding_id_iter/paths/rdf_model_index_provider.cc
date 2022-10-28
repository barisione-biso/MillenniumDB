#include "rdf_model_index_provider.h"

#include "execution/binding_id_iter/paths/btree_path_index_iter.h"
#include "query_optimizer/rdf_model/rdf_model.h"
#include "storage/index/bplus_tree/bplus_tree.h"

using namespace Paths;
using namespace std;

// Rdf Model
RdfModelIndexProvider::RdfModelIndexProvider(// unordered_map<uint64_t, IndexType> t_info,
                                             // unordered_map<uint64_t, IndexType> t_inv_info,
                                             bool* interruption_requested) :
    // t_info     (move(t_info)),
    // t_inv_info (move(t_inv_info)),
    interruption_requested (interruption_requested) {}


bool RdfModelIndexProvider::node_exists(uint64_t node_id) {
    return true;
}


unique_ptr<PathIndexIter> RdfModelIndexProvider::get_btree_iterator(uint64_t type_id, bool inverse, uint64_t node_id) {
    // B+Tree settings
    array<uint64_t, 3> min_ids;
    array<uint64_t, 3> max_ids;
    min_ids[0] = type_id;
    max_ids[0] = type_id;
    min_ids[1] = node_id;
    max_ids[1] = node_id;
    min_ids[2] = 0;
    max_ids[2] = UINT64_MAX;
    min_ids[3] = 0;
    max_ids[3] = UINT64_MAX;

    // Get iter from correct B+Tree
    if (inverse) {
        return make_unique<BTreePathIndexIter>(
            rdf_model.pos->get_range(interruption_requested,
                                     Record<3>(min_ids),
                                     Record<3>(max_ids)));
    } else {
        return make_unique<BTreePathIndexIter>(
            rdf_model.pso->get_range(interruption_requested,
                                     Record<3>(min_ids),
                                     Record<3>(max_ids)));
    }
}

unique_ptr<PathIndexIter> RdfModelIndexProvider::get_iterator(uint64_t predicate_id, bool inverse, uint64_t node_id) {
    return get_btree_iterator(predicate_id, inverse, node_id);
}
