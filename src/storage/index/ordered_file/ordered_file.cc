#include "ordered_file.h"

#include <experimental/filesystem>
#include <chrono>
#include <climits>
#include <iostream>
#include <memory>

#include "storage/index/record.h"
#include "storage/file_manager.h"

using namespace std;

template <std::size_t N>
OrderedFile<N>::OrderedFile(const string& filename) :
    file_id(file_manager.get_file_id(filename)),
    tmp_file_id(file_manager.get_file_id(filename + ".tmp")),
    file(file_manager.get_file(file_id)),
    tmp_file(file_manager.get_file(tmp_file_id))
{
    big_buffer = new uint64_t[(MAX_RUNS+1)*TUPLES_PER_BLOCK*N];
    buffer = new uint64_t*[MAX_RUNS];

    output_buffer = &big_buffer[0];
    for (uint_fast32_t i = 0; i < MAX_RUNS; i++) {
        buffer[i] = &big_buffer[(i+1)*TUPLES_PER_BLOCK*N];
    }
    current_output_pos = 0;
}


template <std::size_t N>
OrderedFile<N>::~OrderedFile() {
    delete[] buffer;
    delete[] big_buffer;
    file_manager.close(file_id);
    file_manager.remove(file_id);
}


template <std::size_t N>
void OrderedFile<N>::begin() {
    file.seekg(0, ios::end);
    filesize = file.tellg();
    file.seekg(0, ios::beg);
}


template <std::size_t N>
bool OrderedFile<N>::has_more_tuples() {
    return file.tellg() < filesize;
}


template <std::size_t N>
uint64_t OrderedFile<N>::get_total_tuples() {
    return filesize / bytes_per_tuple;
}

template <std::size_t N>
uint_fast32_t OrderedFile<N>::next_tuples(uint64_t* output, uint_fast32_t max_tuples) {
    file.read(reinterpret_cast<char*>(output), max_tuples*bytes_per_tuple);
    auto res = file.gcount()/bytes_per_tuple;
    file.clear(); // clear posible badbit

    return res;
}


template <std::size_t N>
std::unique_ptr<Record<N>> OrderedFile<N>::next_record() {
    std::array<uint64_t, N> arr;
    file.read(reinterpret_cast<char*>(arr.data()), bytes_per_tuple);

    auto readed = file.gcount()/bytes_per_tuple;
    file.clear(); // clear posible badbit

    if (readed == 0) {
        return nullptr;
    } else {
        return make_unique<Record<N>>(move(arr));
    }
}


template <std::size_t N>
void OrderedFile<N>::append_record(const Record<N>& record) {
    for (uint_fast32_t col = 0; col < N; col++) {
        output_buffer[current_output_pos*N+col] = record.ids[col];
    }
    current_output_pos++;
    if (current_output_pos == TUPLES_PER_BLOCK) {
        file.seekg(0, ios::end);
        file.write((char*) output_buffer, block_size_in_bytes);
        current_output_pos = 0;
    }
}


template <std::size_t N>
void OrderedFile<N>::order(std::array<uint_fast8_t, N> column_order) {
    file_manager.ensure_open(tmp_file_id);

    if (current_output_pos > 0) {
        file.write((char*) output_buffer, bytes_per_tuple*current_output_pos);
        current_output_pos = 0;
    }

    // auto start = std::chrono::system_clock::now();
    bool reading_orginal_file = true;

    file.seekg(0, ios::end);
    const uint64_t size_in_bytes = file.tellg();
    uint_fast32_t total_blocks = size_in_bytes/block_size_in_bytes + (size_in_bytes%block_size_in_bytes != 0);

    bool reorder = false;
    for (size_t i = 0; i < column_order.size(); i++) {
        if (column_order[i] != i) {
            reorder = true;
            break;
        }
    }

    // Order chunks of size (MAX_RUNS) blocks
    for (uint_fast32_t i = 0; i < total_blocks; /*i+= MAX_RUNS*/i++) {
        create_run(big_buffer, i, column_order, reorder);
    }

    // auto end_phase0 = std::chrono::system_clock::now();
    // std::chrono::duration<float,std::milli> duration = end_phase0 - start;
    // std::cout << duration.count() << "ms " << "\n";

    uint_fast32_t buffer_size[MAX_RUNS];
    uint_fast32_t buffer_current_pos[MAX_RUNS];
    uint_fast32_t buffer_current_block[MAX_RUNS];

    // MERGE ITERATION
    for (uint_fast32_t merge_size = 1; merge_size < total_blocks; merge_size *= MAX_RUNS) {
        file.seekg(0, ios::beg);
        tmp_file.seekg(0, ios::beg);
        std::fstream& file_reading = reading_orginal_file? file : tmp_file;
        std::fstream& file_writing = reading_orginal_file? tmp_file : file;

        for (uint_fast32_t i = 0; i < total_blocks; i+= merge_size*MAX_RUNS) {
            file_reading.seekg(i*block_size_in_bytes, ios::beg);
            // SET HOW MANY RUNS ARE NEEDED
            uint_fast32_t runs = (total_blocks < (i + (merge_size*MAX_RUNS))) ?
                ((total_blocks - i)/merge_size + ((total_blocks - i)%merge_size != 0)) :
                MAX_RUNS;

            // FILL BUFFERS
            for (uint_fast32_t run = 0; run < runs; run++) {
                buffer_current_pos[run] = 0;
                buffer_current_block[run] = 0;
                file_reading.seekg((i + (run*merge_size)) * block_size_in_bytes, ios::beg);
                file_reading.read((char*)buffer[run], block_size_in_bytes);
                buffer_size[run] = file_reading.gcount()/bytes_per_tuple;
                file_reading.clear(); // clear posible badbit
            }

            // merge runs
            while (true) {
                int min_run = -1;
                for (uint_fast32_t run = 0; run < runs; run++) {
                    if (buffer_current_pos[run] < buffer_size[run]) { // check run is not empty
                        if (min_run == -1) {
                            min_run = run;
                        } else {
                            // compare current record at min_run vs record at run
                            for (uint_fast8_t col = 0; col < N; col++/*uint_fast8_t col : column_order*/) {
                                if (buffer[run][buffer_current_pos[run]*N+col] < buffer[min_run][buffer_current_pos[min_run]*N+col]) {
                                    min_run = run;
                                    break;
                                } else if (buffer[run][buffer_current_pos[run]*N+col] > buffer[min_run][buffer_current_pos[min_run]*N+col]) {
                                    break;
                                }
                            }
                        }
                    }
                }
                if (min_run == -1) {
                    break;
                }
                // write current min_record to buffer
                for (uint_fast32_t col = 0; col < N; col++) {
                    output_buffer[current_output_pos*N+col] = buffer[min_run][buffer_current_pos[min_run]*N+col];
                }
                current_output_pos++;
                buffer_current_pos[min_run]++;

                // if output_buffer is full, write to file
                if (current_output_pos == TUPLES_PER_BLOCK) {
                    current_output_pos = 0;
                    file_writing.write((char*) output_buffer, block_size_in_bytes);
                }

                // check if buffer[min_run] is empty
                if (buffer_current_pos[min_run] == buffer_size[min_run]) {
                    buffer_current_block[min_run]++;
                    if (buffer_current_block[min_run] < merge_size) { // if run has another block
                        buffer_current_pos[min_run] = 0;
                        file_reading.seekg((i + (min_run*merge_size) + buffer_current_block[min_run]) * block_size_in_bytes, ios::beg);
                        file_reading.read((char*)buffer[min_run], block_size_in_bytes);
                        buffer_size[min_run] = file_reading.gcount()/bytes_per_tuple;
                        file_reading.clear(); // clear posible badbit
                    }
                }
            }
            // last block is not written unless it was full at last iteration
            if (current_output_pos > 0) {
                file_writing.write((char*) output_buffer, current_output_pos*bytes_per_tuple);
                current_output_pos = 0;
            }
        }
        reading_orginal_file = !reading_orginal_file;
    }

    if (!reading_orginal_file) {
        // file_manager.remove(file_id);
        file_manager.close(file_id);
        std::remove( file_manager.get_absolute_path(file_id).c_str() );

        // file_manager.rename(tmp_file_id, file_manager.get_absolute_path(file_id) );
        file_manager.close(tmp_file_id);
        experimental::filesystem::rename(file_manager.get_absolute_path(tmp_file_id), file_manager.get_absolute_path(file_id));
    }
    else {
        // file_manager.remove(tmp_file_id);
        file_manager.close(file_id);
        std::remove( file_manager.get_absolute_path(tmp_file_id).c_str() );
    }
    file_manager.ensure_open(file_id);

    // auto end = std::chrono::system_clock::now();
    // duration = end - end_phase0;
    // std::cout << duration.count() << "ms " << "\n";
}


// First Step: order (MAX_RUNS) blocks at once
template <std::size_t N>
void OrderedFile<N>::create_run(uint64_t* buffer, uint_fast32_t block_number,
                                std::array<uint_fast8_t, N>& column_order, bool reorder)
{
    file.seekg(block_number*block_size_in_bytes, ios::beg);
    file.read((char*)buffer, block_size_in_bytes/**MAX_RUNS*/);
    uint_fast32_t bytes_readed = file.gcount();
    uint_fast32_t tuples = bytes_readed / bytes_per_tuple;
    file.clear(); // clear posible badbit

    uint64_t* key = new uint64_t[N];
    if (reorder) {
        // reorder according to column_order
        for (uint_fast32_t i = 0; i < tuples; i++) {
            for (uint_fast8_t n = 0; n < N; n++) {
                key[column_order[n]] = buffer[i*N + n];
            }
            for (uint_fast8_t n = 0; n < N; n++) {
                buffer[i*N + n] = key[n];
            }
        }
    }

    // insertion sort
    for (uint_fast32_t i = 1; i < tuples; i++) {
        for (uint_fast8_t n = 0; n < N; n++) {
            key[n] = buffer[i*N + n];
        }
        uint_fast32_t j = i - 1;
        while (record_less_than(j, key, buffer))
        {
            move_record_right(j, buffer);
            if (j == 0) { // necesary to check this because j is unsigned
                j--;
                break;
            }
            j--;
        }
        // insert key at j+1
        assign_record(key, j+1, buffer); // arr[j + 1] = key;
    }
    file.seekg(block_number*block_size_in_bytes, ios::beg);
    file.write((char*)buffer, bytes_readed);
    delete[] key;
}


// returns true if key is less than the record in the buffer at buffer_pos
template <std::size_t N>
bool OrderedFile<N>::record_less_than(uint_fast32_t buffer_pos, uint64_t* key, uint64_t* buffer) {
    for (uint_fast8_t col = 0; col < N; col++) {
        if (key[col] < buffer[buffer_pos*N + col]) {
            return true;
        }
        else if (key[col] > buffer[buffer_pos*N + col]) {
            return false;
        }
    }
    return false;// they are equal
}


template <std::size_t N>
void OrderedFile<N>::move_record_right(uint_fast32_t buffer_pos, uint64_t* buffer) {
    for (uint_fast8_t i = 0; i < N; i++) {
        buffer[(buffer_pos+1)*N + i] = buffer[buffer_pos*N + i];
    }
}


template <std::size_t N>
void OrderedFile<N>::assign_record(uint64_t* key, uint_fast32_t buffer_pos, uint64_t* buffer) {
    for (uint_fast8_t i = 0; i < N; i++) {
        buffer[buffer_pos*N + i] = key[i];
    }
}


template <std::size_t N>
void OrderedFile<N>::print() {
    std::cout << "printing\n";
    file.seekg(0, ios::beg);
    int count = 1;

    uint64_t* buffer = new uint64_t[N];
    file.read((char*)buffer, bytes_per_tuple);
    auto a = file.gcount();
    while (a) {
        std::cout << count++;
        std::cout << " " << a;
        for (uint_fast32_t i = 0; i < N; i++) {
            std::cout << "\t" << buffer[i];
        }
        std::cout << "\n";
        file.read((char*)buffer, bytes_per_tuple);
        a = file.gcount();
    }
    file.clear(); // clear badbit?
    delete[] buffer;
}


template <std::size_t N>
void OrderedFile<N>::check_order() {
    std::cout << "checking order...\n";
    file.seekg(0, ios::beg);

    uint64_t* buffer = new uint64_t[N];
    file.read((char*)buffer, bytes_per_tuple);

    std::array<uint64_t, N> arrA;
    for (uint_fast32_t i = 0; i < N; ++i) {
        arrA[i] = buffer[i];
    }

    auto recordA = Record<N>(move(arrA));

    file.read((char*)buffer, bytes_per_tuple);
    auto a = file.gcount();
    std::array<uint64_t, N> arrB;
    for (uint_fast32_t i = 0; i < N; ++i) {
        arrB[i] = buffer[i];
    }

    Record recordB = Record(move(arrB));

    int count = 2;
    while (a) {
        if (!(recordA < recordB)) {
            cerr << "Bad ordering at tuple " << count << "\n";
            for (uint_fast32_t i = 0; i < N; i++) {
                cout << recordA.ids[i] << "\t";
            }
            cerr << " > ";
            for (uint_fast32_t i = 0; i < N; i++) {
                cerr << recordB.ids[i] << "\t";
            }
            cerr << "\n";
            // exit(-1);
        }
        recordA = recordB;

        file.read((char*)buffer, bytes_per_tuple);
        a = file.gcount();
        std::array<uint64_t, N> arr;
        for (uint_fast32_t i = 0; i < N; ++i) {
            arr[i] = buffer[i];
        }
        recordB = Record(move(arr));
        count++;
    }
    file.clear(); // clear badbit?
    delete[] buffer;
}
