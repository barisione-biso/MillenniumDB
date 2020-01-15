#include "projection.h"

using namespace std;

// Projection::Projection(vector<VarId> vars)
//     : vars(move(vars))
// {}




// unique_ptr<BindingId> Projection::next() {
//     auto next = iter->next();
//     if (next == nullptr) {
//         return nullptr;
//     }
//     else {
//         auto projected = make_unique<BindingId>(vars.size());
//         // TODO: add corresponding
//         for (auto& var_id : vars) {
//             projected->add(var_id, (*next)[var_id]);
//         }
//         return projected;
//     }
// }
