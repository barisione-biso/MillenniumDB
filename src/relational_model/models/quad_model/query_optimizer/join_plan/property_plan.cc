#include "property_plan.h"

#include <cassert>

#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

PropertyPlan::PropertyPlan(QuadModel& model, Id object, Id key, Id value) :
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
        cout << "object: " << model.get_graph_object(std::get<ObjectId>(object)) << "";
    } else {
        cout << "object: " <<  var_names[std::get<VarId>(object).id] << "";
    }

    if (std::holds_alternative<ObjectId>(key)) {
        cout << ", key: " << model.get_graph_object(std::get<ObjectId>(key)) << "";
    } else {
        cout << ", key: " <<  var_names[std::get<VarId>(key).id] << "";
    }

    if (std::holds_alternative<ObjectId>(value)) {
        cout << ", value: " << model.get_graph_object(std::get<ObjectId>(value)) << "";
    } else {
        cout << ", value: " <<  var_names[std::get<VarId>(value).id] << "";
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
unique_ptr<BindingIdIter> PropertyPlan::get_binding_id_iter(std::size_t binding_size) {
    array<unique_ptr<ScanRange>, 3> ranges;

    assert((key_assigned || !value_assigned) && "fixed values with open key is not supported");

    if (object_assigned) {
        ranges[0] = get_scan_range(object, object_assigned);
        ranges[1] = get_scan_range(key, key_assigned);
        ranges[2] = get_scan_range(value, value_assigned);
        return make_unique<IndexScan<3>>(binding_size, *model.object_key_value, move(ranges));
    } else {
        ranges[0] = get_scan_range(key, key_assigned);
        ranges[1] = get_scan_range(value, value_assigned);
        ranges[2] = get_scan_range(object, object_assigned);
        return make_unique<IndexScan<3>>(binding_size, *model.key_value_object, move(ranges));
    }
}
