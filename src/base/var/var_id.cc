#include "base/var/var_id.h"

// VarId VarId::term = VarId(-1);

VarId::VarId(int_fast32_t id)
    : id(id)
{
}

// bool VarId::is_null() {
//     return var_id == -1;
// }