#include "projection.h"

#include "file/index/object_file/object_file.h"

using namespace std;

Projection::Projection(ObjectFile& obj_file, unique_ptr<BindingIdIter> iter,
        std::map<std::string, std::string> constants, vector<string> names,
        vector<int_fast32_t> var_positions, int_fast32_t global_vars)
    : obj_file(obj_file), iter(move(iter)), constants(move(constants)),
      names(move(names)), var_positions(move(var_positions)), global_vars(global_vars)
{
    // TODO: if checking flag, assure names.size() == var_positions.size()
    // and each element
}


void Projection::init() {
    root_input = make_unique<BindingId>(global_vars);
    iter->init(*root_input);
}


std::unique_ptr<Binding> Projection::next() {
    auto next = iter->next();
    if (next == nullptr) {
        return nullptr;
    }
    else {
        auto projected = make_unique<Binding>(); // TODO: ver forma mas eficiente y eliminar memory leak

        int_fast32_t i = 0;
        for (auto& var_position : var_positions) {
            if (var_position >= 0) {
                auto id = (*next)[var_position];
                auto value_bytes = obj_file.read(id);
                string value_string(value_bytes->begin(), value_bytes->end()); // TODO: support other types
                projected->add(names[i], value_string);
            }
            else {
                projected->add(names[i], constants[names[i]]);
            }
            i++;
        }
        return projected;
    }
}
