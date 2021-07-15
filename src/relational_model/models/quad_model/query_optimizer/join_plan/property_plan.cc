#include "property_plan.h"

#include <cassert>

#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

PropertyPlan::PropertyPlan(const QuadModel& model, Id object, Id key, Id value) :
    model           (model),
    object          (object),
    key             (key),
    value           (value),
    object_assigned (std::holds_alternative<ObjectId>(object)),
    key_assigned    (std::holds_alternative<ObjectId>(key)),
    value_assigned  (std::holds_alternative<ObjectId>(value)) { }


PropertyPlan::PropertyPlan(const PropertyPlan& other) :
    model           (other.model),
    object          (other.object),
    key             (other.key),
    value           (other.value),
    object_assigned (other.object_assigned),
    key_assigned    (other.key_assigned),
    value_assigned  (other.value_assigned) { }


std::unique_ptr<JoinPlan> PropertyPlan::duplicate() {
    return make_unique<PropertyPlan>(*this);
}


void PropertyPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "Property(";
    if (std::holds_alternative<ObjectId>(object)) {
        cout << "object: " << model.get_graph_object(std::get<ObjectId>(object));
    } else {
        cout << "object: " <<  var_names[std::get<VarId>(object).id];
    }

    if (std::holds_alternative<ObjectId>(key)) {
        cout << ", key: " << model.get_graph_object(std::get<ObjectId>(key));
    } else {
        cout << ", key: " <<  var_names[std::get<VarId>(key).id];
    }

    if (std::holds_alternative<ObjectId>(value)) {
        cout << ", value: " << model.get_graph_object(std::get<ObjectId>(value));
    } else {
        cout << ", value: " <<  var_names[std::get<VarId>(value).id];
    }

    cout << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}


double PropertyPlan::estimate_cost() {
    return /*100.0 +*/ estimate_output_size();
}


double PropertyPlan::estimate_output_size() {
    const auto total_objects    = static_cast<double>(model.catalog().identifiable_nodes_count
                                                    + model.catalog().anonymous_nodes_count
                                                    + model.catalog().connections_count);

    const auto total_properties = static_cast<double>(model.catalog().properties_count);

    assert((key_assigned || !value_assigned) && "fixed values with open key is not supported");

    if (total_objects == 0) { // To avoid division by 0
        return 0;
    }

    if (key_assigned) {
        double distict_values;
        double key_count;
        if (std::holds_alternative<ObjectId>(key)) {
            distict_values = static_cast<double>(model.catalog().key2distinct[std::get<ObjectId>(key).id]);
            key_count      = static_cast<double>(model.catalog().key2total_count[std::get<ObjectId>(key).id]);
        } else {
            // TODO: this case is not possible yet, but we need to cover it for the future
            return 0;
        }

        if (distict_values == 0) { // To avoid division by 0
            return 0;
        }
        if (value_assigned) {
            if (object_assigned) {
                return key_count / (distict_values * total_objects);
            } else {
                return key_count / distict_values;
            }
        } else {
            if (object_assigned) {
                return key_count / total_objects;
            } else {
                return key_count;
            }
        }
    } else {
        if (object_assigned) {
            return total_properties / total_objects; // key and value not assigned
        } else {
            return total_properties; // nothing assigned
        }
    }
}


void PropertyPlan::set_input_vars(const uint64_t input_vars) {
    if (std::holds_alternative<VarId>(object)) {
        auto object_var_id = std::get<VarId>(object);
        if ((input_vars & (1UL << object_var_id.id)) != 0) {
            object_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(key)) {
        auto key_var_id = std::get<VarId>(key);
        if ((input_vars & (1UL << key_var_id.id)) != 0) {
            key_assigned = true;
        }
    }

    if (std::holds_alternative<VarId>(value)) {
        auto value_var_id = std::get<VarId>(value);
        if ((input_vars & (1UL << value_var_id.id)) != 0) {
            value_assigned = true;
        }
    }
}

uint64_t PropertyPlan::get_vars() {
    uint64_t result = 0;

    if ( std::holds_alternative<VarId>(object) ) {
        result |= 1UL << std::get<VarId>(object).id;
    }
    if ( std::holds_alternative<VarId>(key) ) {
        result |= 1UL << std::get<VarId>(key).id;
    }
    if ( std::holds_alternative<VarId>(value) ) {
        result |= 1UL << std::get<VarId>(value).id;
    }
    return result;
}


/**
 * ╔═╦══════════════╦═══════════════╦══════════════════╦═════════╗
 * ║ ║  KeyAssigned ║ ValueAssigned ║  ObjectAssigned  ║  Index  ║
 * ╠═╬══════════════╬═══════════════╬══════════════════╬═════════╣
 * ║1║      yes     ║      yes      ║        yes       ║   OKV   ║
 * ║2║      yes     ║      yes      ║        no        ║   KVO   ║
 * ║3║      yes     ║      no       ║        yes       ║   OKV   ║
 * ║4║      yes     ║      no       ║        no        ║   KVO   ║
 * ║5║      no      ║      yes      ║        yes       ║  ERROR  ║
 * ║6║      no      ║      yes      ║        no        ║  ERROR  ║
 * ║7║      no      ║      no       ║        yes       ║   OKV   ║
 * ║8║      no      ║      no       ║        no        ║   KVO   ║
 * ╚═╩══════════════╩═══════════════╩══════════════════╩═════════╝
 */
unique_ptr<BindingIdIter> PropertyPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 3> ranges;

    assert((key_assigned || !value_assigned) && "fixed values with open key is not supported");

    if (object_assigned) {
        ranges[0] = ScanRange::get(object, object_assigned);
        ranges[1] = ScanRange::get(key, key_assigned);
        ranges[2] = ScanRange::get(value, value_assigned);
        return make_unique<IndexScan<3>>(*model.object_key_value, move(ranges));
    } else {
        ranges[0] = ScanRange::get(key, key_assigned);
        ranges[1] = ScanRange::get(value, value_assigned);
        ranges[2] = ScanRange::get(object, object_assigned);
        return make_unique<IndexScan<3>>(*model.key_value_object, move(ranges));
    }
}


unique_ptr<LeapfrogIter> PropertyPlan::get_leapfrog_iter(const std::set<VarId>& assigned_vars,
                                                         const vector<VarId>&   var_order,
                                                         uint_fast32_t          enumeration_level)
{
    vector<unique_ptr<ScanRange>> initial_ranges;
    vector<VarId> intersection_vars;
    vector<VarId> enumeration_vars;

    // index = INT32_MAX means enumeration, index = -1 means term
    int_fast32_t obj_index, key_index, value_index;

    // Assign obj_index
    if (std::holds_alternative<ObjectId>(object)) {
        obj_index = -1;
    } else {
        auto search = assigned_vars.find(std::get<VarId>(object));
        if (search == assigned_vars.end()) {
            obj_index = INT32_MAX;
        } else {
            obj_index = -1;
        }
    }

    // Assign key_index
    if (std::holds_alternative<ObjectId>(key)) {
        key_index = -1;
    } else {
        auto search = assigned_vars.find(std::get<VarId>(key));
        if (search == assigned_vars.end()) {
            key_index = INT32_MAX;
        } else {
            key_index = -1;
        }
    }

    // Assign value_index
    if (std::holds_alternative<ObjectId>(value)) {
        value_index = -1;
    } else {
        auto search = assigned_vars.find(std::get<VarId>(value));
        if (search == assigned_vars.end()) {
            value_index = INT32_MAX;
        } else {
            value_index = -1;
        }
    }

    // search for vars marked as enumeraion (INT32_MAX) that are intersection
    // and assign them the correct index
    for (size_t i = 0; i < enumeration_level; i++) {
        if (obj_index == INT32_MAX && std::get<VarId>(object) == var_order[i]) {
            obj_index = i;
        }
        if (key_index == INT32_MAX && std::get<VarId>(key) == var_order[i]) {
            key_index = i;
        }
        if (value_index == INT32_MAX && std::get<VarId>(value) == var_order[i]) {
            value_index = i;
        }
    }

    auto assign = [&initial_ranges, &enumeration_vars, &intersection_vars]
                  (int_fast32_t& index, Id id)
                  -> void
    {
        if (index == -1) {
            initial_ranges.push_back(ScanRange::get(id, true));
        } else if (index == INT32_MAX) {
            enumeration_vars.push_back(std::get<VarId>(id));
        } else {
            intersection_vars.push_back(std::get<VarId>(id));
        }
    };

    // object_key_value
    if (obj_index <= key_index && key_index <= value_index) {
        assign(obj_index,   object);
        assign(key_index,   key);
        assign(value_index, value);

        return make_unique<LeapfrogBptIter<3>>(
            *model.object_key_value,
            move(initial_ranges),
            move(intersection_vars),
            move(enumeration_vars)
        );
    }
    // key_value_object
    else if (key_index <= value_index && value_index <= obj_index) {
        assign(key_index,   key);
        assign(value_index, value);
        assign(obj_index,   object);

        return make_unique<LeapfrogBptIter<3>>(
            *model.key_value_object,
            move(initial_ranges),
            move(intersection_vars),
            move(enumeration_vars)
        );
    } else {
        return nullptr;
    }
}
