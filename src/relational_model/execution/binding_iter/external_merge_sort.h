/*#ifndef RELATIONAL_MODEL__EXTERNAL_MERGE_SORT_H_
#define RELATIONAL_MODEL__EXTERNAL_MERGE_SORT_H_

#include "base/ids/var_id.h"
#include "base/binding/binding_iter.h"
#include "storage/file_id.h"
#include "storage/page.h"
#include <map>
#include <memory>
#include <vector>


class ExternalMergeSort : public BindingIter {

private:
    //FileId file_phase_0;
    // BindingId* current_left;
    std::unique_ptr<BindingIter> root;
    Binding* current_binding;
    inline void construct_binding();
    //std::vector<FileId> files_phase_1; // guarda los archivos de la fase 1
    //std::vector<uint_fast32_t> tuples_of_file_phase_1; // guarda cuantas tuplas tiene el archivo i de fase 1
    //std::vector<uint_fast32_t> tuples_returned_in_phase_2; // cuantas se han retornado del archivo i de fase 1 en fase 2
    //std::unique_ptr<BindingId> my_binding;
    //BindingId* input_dir;
    //int_fast32_t tuples_in_last_page = 0; // indica cuantas tuplas quedaron en la pagina final de fase 0
    //uint_fast32_t total_tuples = 0;  // cantidad total de tuplas
    //uint_fast32_t tuples_counter = 0; // contador de tuplas
    //uint_fast32_t total_pages = 0;  // contador de paginas
    //int_fast32_t tuple_size = 0;  // tamaño de una tupla
    //std::vector<BindingId> r; // run
    //void phase_0();
    //void phase_1();
    //int partition(int i, int f);
    //void quicksort(int i, int f);
    //void merge(uint_fast32_t B_start, uint_fast32_t B_end);
    //void copy_binding_id(BindingId* destiny, BindingId origin);

public:
    ExternalMergeSort(std::size_t binding_size, std::unique_ptr<BindingIter> root);
    ~ExternalMergeSort() = default;
    void write();
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
};

template class std::unique_ptr<ExternalMergeSort>;

#endif //RELATIONAL_MODEL__EXTERNAL_MERGE_SORT_H_
*/