#include "base/var/var_id.h"

VarId VarId::null_var = VarId(-1);

VarId::VarId(int var_id)
    : var_id(var_id)
{
}

// bool VarId::is_null() {
//     return var_id == -1;
// }