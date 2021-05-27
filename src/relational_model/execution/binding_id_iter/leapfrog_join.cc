#include "leapfrog_join.h"

#include <cassert>

#include "storage/index/tuple_buffer/tuple_buffer.h"

using namespace std;

LeapfrogJoin::LeapfrogJoin(vector<unique_ptr<LeapfrogIter>> leapfrog_iters,
                           vector<VarId> var_order) :
    leapfrog_iters    (move(leapfrog_iters)),
    var_order         (move(var_order)),
    base_level        (0), // TODO: base_level != 0 when having optionals?
    level             (base_level - 1)
    { }


void LeapfrogJoin::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;

    if (!parent_has_next) {
        // level = base_level - 1 so next() will return false
        return;
    }

    // TODO: para optionals, si base_level > 0, ver parent_binding y setear iteradores correspondientes?

    // initialize iters_for_var and enumeration_level
    for (size_t i = 0; i < var_order.size(); i++) { // TODO: para optionals, si base_level > 0, i = base_level?
        vector<LeapfrogIter*> iter_list;
        for (const auto& leapfrog_iter : leapfrog_iters) {
            // add iter to the list if it contains the var
            for (const auto& intersection_var : leapfrog_iter->get_intersection_vars()) {
                if (intersection_var == var_order[i]) {
                    iter_list.push_back(leapfrog_iter.get());
                    break;
                }
            }
        }
        if (iter_list.size() > 0) {
            iters_for_var.push_back(move(iter_list));
        } else {
            enumeration_level = i;
            break;
        }
    }

    // Initialize 1 buffer per iterator
    for (uint_fast32_t i = 0; i < leapfrog_iters.size(); i++) {
        buffers.push_back(make_unique<TupleBuffer>(leapfrog_iters[i]->get_enumeration_vars()));
    }
    buffer_pos.resize(leapfrog_iters.size());

    // open terms
    bool open_terms = true;
    for (auto& lf_iter : leapfrog_iters) {
        if (!lf_iter->open_terms()) {
            open_terms = false;
            break;
        }
    }

    cout << "base_level: " << base_level << "\n";
    cout << "enumeration_level: " << enumeration_level << "\n";

    if (open_terms) {
        cout << "open_terms successfully\n";
        down();
    } else {
        cout << "open_terms failed\n";
    }
    cout << "level: " << level << "\n";
}


bool LeapfrogJoin::next() {
    // cout << "next called\n";
    // cout << "level: " << level << "\n";

    while (level >= base_level) {
        while (level < enumeration_level) {
            // We try to bind the variable for the current level
            if (find_intersection_for_current_level()) {
                down();
            } else {
                if (level == base_level) {
                    return false;
                } else {
                    up();
                    // We are in a previous intersection, so we need to move the last iterator forward
                    // to avoid having the same intersection
                    if (!iters_for_var[level][iters_for_var[level].size() - 1]->next()) {
                        cout << "TODO: esto puede estar mal\n"; // TODO: porque?
                        return false;
                    }
                }
            }
        }
        // TODO: me pude saltar el llenado de buffers, ej: MATCH (?y :forsworn) base_level = 0 y enumeration_level = 0
        // at this point level == enumeration_level
        // check if enumeration is not over
        for (size_t i = 0; i < buffers.size(); i++) {
            if (buffer_pos[i]+1 < (int_fast32_t) buffers[i]->get_tuple_count()) {
                buffer_pos[i]++;
                buffers[i]->assign_to_binding(*parent_binding, buffer_pos[i]);

                while (i > 0) {
                    i--;
                    buffer_pos[i] = 0;
                    buffers[i]->assign_to_binding(*parent_binding, buffer_pos[i]);
                }
                // set binding for enumerating vars
                return true;
            }
        }

        // when enumeration is over backtrack to previous level
        level--; // go back to last enumeration_level

        if (iters_for_var.size() == 0) {
            return false;
        } else {
            while (level >= base_level
                   && !iters_for_var[level][iters_for_var[level].size() -  1]->next())
            {
                up();
            }
        }
    }
    return false;
}


void LeapfrogJoin::reset() {
    while (level >= 0) { // TODO: when using optionals level should start at base_level?
        up();
    }
    down();
}


void LeapfrogJoin::up() {
    assert (level >= 0);
    for (uint_fast32_t i = 0; i < iters_for_var[level].size(); i++) {
        iters_for_var[level][i]->up();
    }
    level--;
}


void LeapfrogJoin::down() {
    assert(level < enumeration_level);
    level++;

    if (level < enumeration_level) {
        for (uint_fast32_t i = 0; i < iters_for_var[level].size(); i++) {
            iters_for_var[level][i]->down();
        }

        // sort the corresponding iterators using insertion sort
        for (int_fast32_t i = 1; i < (int_fast32_t) iters_for_var[level].size(); i++) {
            auto aux = iters_for_var[level][i];
            int_fast32_t j;
            for (j = i - 1; j >= 0 && iters_for_var[level][j]->get_key() > aux->get_key(); j--) {
                iters_for_var[level][j + 1] = iters_for_var[level][j];
            }
            iters_for_var[level][j + 1] = aux;
        }
    } else { // level == enumeration_level
        // prepare for enumeration phase
        for (uint_fast32_t i = 0; i < leapfrog_iters.size(); i++) {
            // consume all results and save them in a buffer
            leapfrog_iters[i]->enum_no_intersection(*buffers[i]);
            buffer_pos[i] = 0;
        }
        for (uint_fast32_t i = 1; i < leapfrog_iters.size(); i++) {
            buffers[i]->assign_to_binding(*parent_binding, 0);
        }
        buffer_pos[0] = -1; // so when calling buffer_pos[0]++ it will be at 0
        // TODO: does sorting by buffer size improve performance?
    }
}



bool LeapfrogJoin::find_intersection_for_current_level() {
    // cout << "trying to find intersection at level " << level << "\n";
    uint_fast32_t p = 0;

    auto min = iters_for_var[level][p]->get_key();
    auto max = iters_for_var[level][iters_for_var[level].size() - 1]->get_key();

    // cout << "min: " << min << "\n";
    // cout << "max: " << max << "\n";

    while (min != max) { // min = max means all are equal
        if (iters_for_var[level][p]->seek(max)) {
            // after the seek, the previous min is the max
            max = iters_for_var[level][p]->get_key();

            // update the min
            p = (p + 1) % iters_for_var[level].size();
            min = iters_for_var[level][p]->get_key();
            // cout << "new min: " << min << "\n";
            // cout << "new max: " << max << "\n";

        } else {
            // cout << "failed to find intersection at level " << level << "\n";
            return false;
        }
    }
    parent_binding->add(var_order[level], ObjectId(min));
    // cout << "found intersection at level " << level << "\n";
    return true;
}


void LeapfrogJoin::analyze(int indent) const {
    cout << std::string(indent, ' ');
    // TODO: put some useful stats
    cout << "LeapfrogJoin()";
}


void LeapfrogJoin::assign_nulls() {
    for (uint_fast32_t lvl = base_level; lvl < var_order.size(); lvl++) {
        parent_binding->add(var_order[lvl], ObjectId::get_null());
    }
}
