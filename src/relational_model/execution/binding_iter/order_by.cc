#include "order_by.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/tuple_collection/tuple_collection.h"


bool is_lt(std::vector<uint64_t> a, std::vector<uint64_t> b, std::vector<uint64_t> order_vars) {
    for (size_t i = 0; i < order_vars.size(); i++) {
        if (a[i] < b[i]) {
            return true;
        }
        if (b[i] < a[i]) {
            return false;
        }
    }
    return true;
}

bool is_gt(std::vector<uint64_t> a, std::vector<uint64_t> b, std::vector<uint64_t> order_vars) {
    for (size_t i = 0; i < order_vars.size(); i++) {
        if (a[i] > b[i]) {
            return true;
        }
        if (b[i] > a[i]) {
            return false;
        }
    }
    return true;
}

using namespace std;

OrderBy::OrderBy(std::unique_ptr<BindingIter> _child,
                   vector<pair<string, VarId>> order_vars,
                   size_t binding_size,
                   const bool ascending) :
    child       (move(_child)),
    order_vars (move(order_vars)),
    my_binding (BindingOrderBy(move(order_vars), child->get_binding())),
    file_id (file_manager.get_file_id("temp.txt"))
{
    n_pages = 0;
    bool (*has_priority)(std::vector<uint64_t> a, std::vector<uint64_t>b, std::vector<uint64_t> order_v) = (ascending) ? is_gt : is_lt;
    std::vector<uint64_t> order_ids = std::vector<uint64_t>(order_vars.size());
    for (size_t i = 0; i < order_vars.size(); i++) {
        order_ids[i] = order_vars[i].second.id;
    }
    TupleCollection run(buffer_manager.get_page(file_id, n_pages), binding_size);
    std::vector<uint64_t> binding_id_vec = std::vector<uint64_t>(binding_size);
    while (child->next()) {
        if (run.is_full()) {
            n_pages++;
            max_tuples = run.get_n_tuples();
            //run.sort(has_priority, order_ids);
            TupleCollection run(buffer_manager.get_page(file_id, n_pages), binding_size);
        }
        for (size_t i = 0; i < binding_size; i++) {
            binding_id_vec[i] = my_binding.get_id(VarId(i)).id;
        }
        run.add(binding_id_vec);
        total_tuples++;
  }
}


Binding& OrderBy::get_binding() {
    return my_binding;
}


bool OrderBy::next() {
  // TODO: CAMBIAR ESTA LOGICA EN NEXT. DEBE COMUNICARSE CON TUPLE_COLLECTION.
  cout << total_tuples << "\n";
  cout << n_pages << "\n";
  cout << max_tuples << "\n";
  if (child->next()) {
    return true;
  }
  return false;
}

void OrderBy::analyze(int indent) const {
    child->analyze(indent);
}


/* TODO: Hacer quicksort a la pagina, evitar el uso de r
void OrderBy::phase_0(){
    // Manda todas las páginas a disco 
    while (current_left != nullptr) {
        r = std::vector<BindingId>();
        // se realiza el llenado del run
        for (int_fast32_t i = 0; (i + 1) * tuple_size <= PAGE_SIZE; i++) {
            if (current_left == nullptr) {
                break;
            }
            r.push_back(*current_left);
            current_left = left->next();
        }
        // se ordena el run usando quicksort
        quicksort(0, r.size() - 1);
        Page& page = buffer_manager.get_page(file_phase_0, total_pages);
        char* start_pointer = page.get_bytes();
        // se lleva el run a disco
        for (size_t i = 0; i < r.size();i++) {
            int tuple_pointer = tuple_size * i;
            uint_fast64_t obj_id = 0;
            for(int_fast32_t j = 0; j < tuple_size / 6; j++) {
                obj_id = r[i][j].id;
                memcpy(start_pointer + tuple_pointer + (j*6), &obj_id, 6);
            }
        }
        total_tuples += r.size();
        tuples_in_last_page = r.size();
        page.make_dirty();
        total_pages++;
    }
}
*/


/* TODO: En un futuro: Implementar carga de a 2 runs
void OrderBy::phase_1(){
    //Se escoge dinámicamente el run
    int B = (int)ceil(sqrt((double) total_pages));
    uint_fast32_t start = 0;
    uint_fast32_t end = B;
    while (end <= total_pages) {
        merge(start,end);
        start+=B;
        end+=B;
        if (end > total_pages && start < total_pages) {
            end = total_pages;
            merge(start,end);
            return;
        }
    }
}
*/


/*TODO: Poco prioritario: Probar con insertion sort y medir tiempos
void OrderBy::quicksort(int i, int f) {
    if (i <= f) {
        int p = partition(i, f);
        quicksort(i, p - 1);
        quicksort(p + 1, f);
    }
}
*/

/*
int OrderBy::partition(int i, int f) {
    // partition implementado según EL ramo de Estructura de Datos
    int x = i + (rand() % (f - i + 1));
    BindingId binding_p = BindingId(my_binding->var_count());
    // p = arreglo[x]
    binding_p = r[x];
    // intercambio de elementos de x y f
    std::swap(r[x], r[f]);
    r[f] = binding_p;
    int j = i;
    BindingId binding_k = BindingId(my_binding->var_count());
    int k = i;
    BindingId aux = BindingId(my_binding->var_count());
    while(k <= f) {
        if (r[k] < binding_p) {
            // intercambiar elemento j con elemento k
            std::swap(r[j], r[k]);
            j++;
        }
        k++;
    }
    std::swap(r[j], r[f]);
    return j;
   return 0;
}
*/

/*
void OrderBy::merge(uint_fast32_t B_start, uint_fast32_t B_end) {
  
    files_phase_1.push_back(file_manager.get_tmp_file_id()); // archivo donde se guardará el output
    tuples_returned_in_phase_2.push_back(0);
    int_fast32_t tuples_per_page = PAGE_SIZE / tuple_size; // tuplas que caben en una pagina
    uint_fast32_t p_index[B_end - B_start];
    for (uint_fast32_t i = 0; i < B_end - B_start; i++){
        p_index[i] = 0; // indica el indice de donde se debe leer la tupla en cada pagina
    }
    bool has_elements = true;
    BindingId min_bin = BindingId(my_binding->var_count());
    r = std::vector<BindingId>();
    uint_fast32_t p_min = 0;  // indica la pagina en la que se encontro el minimo
    while (has_elements) {
        has_elements = false;
        for (uint_fast32_t i = B_start; i < B_end; i++) { // se busca el minimo en el primer indice de la pagina
            if(((i == total_pages - 1) && (p_index[i - B_start] >= tuples_in_last_page)) || (p_index[i - B_start] == tuples_per_page)) {
                // evita seguir buscando si ya se llegó al final de la pagina
                continue;
            }
            int tuple_pointer = tuple_size * p_index[i - B_start];
            Page& page = buffer_manager.get_page(file_phase_0, i);
            char* start_pointer = page.get_bytes();
            uint_fast64_t obj_id = 0;
            BindingId aux = BindingId(my_binding->var_count());
            for(int_fast32_t j = 0; j < tuple_size / 6; j++) {
                memcpy(&obj_id, start_pointer + tuple_pointer + j*6, 6);
                aux.add(VarId(j),ObjectId(obj_id));
            }
            if (!has_elements || aux < min_bin) {
                copy_binding_id(&min_bin, aux);
                has_elements = true;
                p_min = i - B_start;
            }
            buffer_manager.unpin(page);
        }
        if (has_elements) {
            p_index[p_min]++; // se avanza el indice en dicha pagina
            r.push_back(min_bin);
        }
    }
    uint_fast32_t p_counter = 0; // contador de paginas
    int index_writting = 0; // indice que indica que indica donde escribir en el archivo de fase 1
    // se envian a disco
    for (size_t i = 0;i < r.size();i++) {
        int tuple_pointer = tuple_size * index_writting;
        Page& page = buffer_manager.get_page(files_phase_1[files_phase_1.size() - 1], p_counter);
        char* start_pointer = page.get_bytes();
        uint_fast64_t obj_id = 0;
        for(int_fast32_t j = 0; j < tuple_size / 6; j++) {
            obj_id = r[i][j].id;
            memcpy(start_pointer + tuple_pointer + j*6, &obj_id, 6);
        }
        index_writting++;
        if (index_writting == tuples_per_page) {
            p_counter++;
            index_writting = 0;
        }
        page.make_dirty();
        buffer_manager.unpin(page);
    }
    tuples_of_file_phase_1.push_back(r.size());
}
*/

/*
void OrderBy::copy_binding_id(BindingId* destiny, BindingId origin) {
    for (size_t i = 0; i < tuple_size / 6; i++) {
        destiny->add(VarId(i),ObjectId(origin[i].id));
    }
}
*/
