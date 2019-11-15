#include "file/index/ordered_file/ordered_file.h"

#include <chrono>
#include <climits>
#include <memory>
#include "file/index/record.h"

#ifndef TUPLES_PER_BLOCK
    #define TUPLES_PER_BLOCK 4096/8
#endif
#ifndef DEFAULT_RUNS
    // #define DEFAULT_RUNS 15
    #define DEFAULT_RUNS 10
#endif

OrderedFile::OrderedFile(const string& filename, uint_fast8_t tuple_size)
    : filename(filename), tuple_size(tuple_size), block_size_in_bytes(TUPLES_PER_BLOCK*sizeof(uint64_t)*tuple_size)
{
    // Open and close to ensure it exists
    fileA.open(filename, std::ios::out | std::ios::app);
    fileA.close();
    fileA.open(filename, fstream::in|fstream::out|fstream::binary);
}


OrderedFile::~OrderedFile() {
    fileA.close();
}

void OrderedFile::append_record(const Record& record) {
    fileA.seekg(0, ios::end);

    for (uint_fast8_t i = 0; i < tuple_size; i++) {
        fileA.write((char *)&record.ids[i], sizeof(uint64_t));
    }
}

void OrderedFile::order(vector<uint_fast8_t> column_order) {
    auto start = std::chrono::system_clock::now();
    fileB.open("test_files/tmp.bin", std::ios::out | std::ios::app);
    fileB.close();
    fileB.open("test_files/tmp.bin", fstream::in|fstream::out|fstream::binary);
    bool readingFileA = true;

    fileA.seekg(0, ios::end);
    const uint64_t size_in_bytes = fileA.tellg();
    uint_fast32_t size_in_blocks = size_in_bytes/block_size_in_bytes + (size_in_bytes%block_size_in_bytes != 0);
    std::cout << "size_in_bytes : " << size_in_bytes << "\n";
    std::cout << "size_in_blocks: " << size_in_blocks << "\n";


    // Reserve buffer memory
    uint64_t* big_buffer = new uint64_t[DEFAULT_RUNS*TUPLES_PER_BLOCK*tuple_size];
    uint64_t* output_buffer = new uint64_t[TUPLES_PER_BLOCK*tuple_size];

    // Order blocks
    for (uint_fast32_t i = 0; i < size_in_blocks; i+= DEFAULT_RUNS) {
        order_block(big_buffer, i, column_order);
    }

    auto end_phase0 = std::chrono::system_clock::now();
    std::chrono::duration<float,std::milli> duration = end_phase0 - start;
    std::cout << duration.count() << "ms " << std::endl;

    // Prepare external merge
    uint_fast32_t merge_size = 1;
    uint_fast32_t runs = DEFAULT_RUNS;
    if (size_in_blocks < DEFAULT_RUNS) {
        runs = size_in_blocks;
    }

    uint64_t* buffer[DEFAULT_RUNS];
    for (uint_fast32_t i = 0; i < DEFAULT_RUNS; i++) {
        buffer[i] = &big_buffer[i*TUPLES_PER_BLOCK*tuple_size];
    }
    uint_fast32_t buffer_size[DEFAULT_RUNS];
    uint_fast32_t buffer_current_pos[DEFAULT_RUNS];
    uint_fast32_t buffer_current_block[DEFAULT_RUNS];

    // MERGE ITERATION
    while (DEFAULT_RUNS*merge_size < size_in_blocks) {
        merge_size *= DEFAULT_RUNS;
	    // std::cout << "Merge Iteration. merge_size : " << merge_size << "\n";

        std::fstream& file_reading = readingFileA? fileA : fileB;
        std::fstream& file_writing = readingFileA? fileB : fileA;
        file_reading.seekg(0, ios::beg);
        file_writing.seekg(0, ios::beg);
        for (uint_fast32_t i = 0; i < size_in_blocks; i+= merge_size*runs) {
            if (i + merge_size >= size_in_blocks) { // No need to sort
                continue;
            }
            // SET HOW MANY RUNS ARE NEEDED
            if (i + (DEFAULT_RUNS*merge_size) > size_in_blocks) {
                runs = (size_in_blocks - i)/merge_size + ((size_in_blocks - i)%merge_size != 0);
            } else {
                runs = DEFAULT_RUNS;
            }
            file_reading.seekg(i*block_size_in_bytes, ios::beg);

            // FILL BUFFERS
            for (uint_fast32_t run = 0; run < runs; run++) { // TODO: mal
                buffer_current_pos[run] = 0;
                buffer_current_block[run] = 0;
                file_reading.seekg((i + (run*merge_size)) * block_size_in_bytes, ios::beg);
                file_reading.read((char*)buffer[run], block_size_in_bytes);
                buffer_size[run] = file_reading.gcount()/(sizeof(uint64_t)*tuple_size);
                file_reading.clear(); // clear posible badbit
            }

            // std::cout << "  merging " << runs << " runs. Blocks from " << i << " to " << (i+(merge_size*runs))<< " \n";
            // (runs)-way merge
            int current_output_pos = 0;

            while (true) { /* while at least 2 buffers are not empty*/
                int min_run = -1;
                for (uint_fast32_t run = 0; run < runs; run++) {
                    if (buffer_current_pos[run] < buffer_size[run]) { // check run is not empty
                        if (min_run == -1) {
                            min_run = run;
                        } else {
                            // compare current record at min_run vs record at run
                            for (uint_fast8_t col : column_order) {
                                if (buffer[run][buffer_current_pos[run]*tuple_size+col] < buffer[min_run][buffer_current_pos[min_run]*tuple_size+col]) {
                                    min_run = run;
                                    break;
                                } else if (buffer[run][buffer_current_pos[run]*tuple_size+col] > buffer[min_run][buffer_current_pos[min_run]*tuple_size+col]) {
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
                for (int col = 0; col < tuple_size; col++) {
                    output_buffer[current_output_pos*tuple_size+col] = buffer[min_run][buffer_current_pos[min_run]*tuple_size+col];
                }
                current_output_pos++;
                buffer_current_pos[min_run]++;

                // if output_buffer is full, write to file
                if (current_output_pos == TUPLES_PER_BLOCK) {
                    current_output_pos = 0;
                    // cout << "write block to file\n";
                    file_writing.write((char*) output_buffer, block_size_in_bytes);
                }

                // check if buffer of min_run is empty
                if (buffer_current_pos[min_run] == buffer_size[min_run]) {
                    // cout << "run " << min_run << " at end of buffer, (" << buffer_size[min_run] << ")\n";
                    buffer_current_block[min_run]++;
                    if (buffer_current_block[min_run] < merge_size /*&&
                        (i + (min_run*merge_size) + buffer_current_block[min_run]) < size_in_blocks*/)
                    { // if run has another block
                        buffer_current_pos[min_run] = 0;
                        file_reading.seekg((i + (min_run*merge_size) + buffer_current_block[min_run]) * block_size_in_bytes, ios::beg);
                        // cout << "Reading " << block_size_in_bytes << " bytes (" << (i + (min_run*merge_size) + buffer_current_block[min_run]) << "): " << file_reading.tellg() << endl;
                        file_reading.read((char*)buffer[min_run], block_size_in_bytes);
                        buffer_size[min_run] = file_reading.gcount()/(sizeof(uint64_t)*tuple_size);
                        file_reading.clear(); // clear posible badbit
                        // cout << "  new buffer size: " << buffer_size[min_run] << "\n";

                    }
                }
            }
            // last block is not written unless it was full at last iteration
            if (current_output_pos > 0) {
                file_writing.write((char*) output_buffer, current_output_pos*tuple_size*sizeof(uint64_t));
            }
        }
        readingFileA = !readingFileA;
    }
    fileB.close();
    if (!readingFileA) {
        // cout << "Changing B to A\n";
        fileA.close();

        remove(filename.c_str());
        // cout << "result remove: " << res << "\n";

        rename("test_files/tmp.bin", filename.c_str());
        fileA.open(filename, fstream::in|fstream::out|fstream::binary);
    }
    else {
        remove("test_files/tmp.bin");
    }
    delete[] big_buffer;
    delete[] output_buffer;

    auto end = std::chrono::system_clock::now();
    duration = end - end_phase0;
    std::cout << duration.count() << "ms " << std::endl;
}

// First Step: order (DEFAULT_RUNS) blocks at once
void OrderedFile::order_block(uint64_t* buffer, uint_fast32_t block_number, vector<uint_fast8_t>& column_order)
{
	// std::cout << "Ordering blocks " << block_number << " to " << (block_number+ DEFAULT_RUNS) << "\n";
    fileA.seekg(block_number*block_size_in_bytes, ios::beg);
    fileA.read((char*)buffer, block_size_in_bytes*DEFAULT_RUNS);
    uint_fast32_t bytes_readed = fileA.gcount();
    uint_fast32_t tuples = bytes_readed / (sizeof(uint64_t)*tuple_size);
    fileA.clear(); // clear posible badbit

    // insertion sort
    for (uint_fast32_t i = 1; i < tuples; i++) {
        vector<uint64_t> vector_key;
        for (uint_fast8_t n = 0; n < tuple_size; n++) {
            vector_key.push_back(buffer[i*tuple_size + n]);
        }
        Record key = Record(vector_key);
        // std::cout << "(" << key.ids[0] << "," << key.ids[1] << ")\n";

        uint_fast32_t j = i - 1;
        while (record_less_than(j, key, buffer, column_order))
        {
            move_record_right(j, buffer);
            if (j == 0) { // necesary to check this because j is unsigned
                j--;
                break;
            }
            j--;
        }
        // insert key at j+1
        assign_record(key, j+1, buffer);//arr[j + 1] = key;
    }
    fileA.seekg(block_number*block_size_in_bytes, ios::beg);
    fileA.write((char*)buffer, bytes_readed);
}

// returns true if key is less than the record in the buffer at buffer_pos
bool OrderedFile::record_less_than(uint_fast32_t buffer_pos, const Record& key, uint64_t* buffer, vector<uint_fast8_t>& column_order)
{
    for (auto column : column_order) {
        if (key.ids[column] < buffer[buffer_pos*tuple_size + column]) {
            return true;
        }
        else if (key.ids[column] > buffer[buffer_pos*tuple_size + column]) {
            return false;
        }
    }
    return false;// they are equal
}

void OrderedFile::move_record_right(uint_fast32_t buffer_pos, uint64_t* buffer)
{
    for (uint_fast8_t i = 0; i < tuple_size; i++) {
        // std::cout << buffer[(buffer_pos+1)*tuple_size + i] << "->" << buffer[buffer_pos*tuple_size + i] << std::endl;
        buffer[(buffer_pos+1)*tuple_size + i] = buffer[buffer_pos*tuple_size + i];
    }
}

void OrderedFile::assign_record(Record& key, uint_fast32_t buffer_pos, uint64_t* buffer)
{
    for (uint_fast8_t i = 0; i < tuple_size; i++) {
        // std::cout << buffer[buffer_pos*tuple_size + i] << "->" << key.ids[i] << std::endl;

        buffer[buffer_pos*tuple_size + i] = key.ids[i];
    }
}

void OrderedFile::print()
{
    std::cout << "printing\n";
    fileA.seekg(0, ios::beg);
    int count = 1;

    uint64_t* buffer = new uint64_t[tuple_size];
    fileA.read((char*)buffer, tuple_size*sizeof(uint64_t));
    auto a = fileA.gcount();
    while (a) {
        std::cout << count++;
        std::cout << " " << a;
        for (int i = 0; i < tuple_size; i++) {
            std::cout << "\t" << buffer[i];
        }
        std::cout << "\n";
        fileA.read((char*)buffer, tuple_size*sizeof(uint64_t));
        a = fileA.gcount();
    }
    fileA.clear(); // clear badbit?
    delete[] buffer;
}

void OrderedFile::check_order(vector<uint_fast8_t> column_order)
{
    std::cout << "checking...\n";
    fileA.seekg(0, ios::beg);

    uint64_t* buffer = new uint64_t[tuple_size];
    fileA.read((char*)buffer, tuple_size*sizeof(uint64_t));

    std::vector<uint64_t> arrA;
    for (auto col : column_order) {
        arrA.push_back(buffer[col]);
    }

    Record recordA = Record(arrA);

    fileA.read((char*)buffer, tuple_size*sizeof(uint64_t));
    auto a = fileA.gcount();
    std::vector<uint64_t> arrB;
    for (auto col : column_order) {
        arrB.push_back(buffer[col]);
    }

    Record recordB = Record(arrB);

    int count = 2;
    while (a) {
        if (!(recordA <= recordB)) {
            cout << "MAL ORDEN. linea " << count << "\n";
            for (int i = 0; i < tuple_size; i++) {
                cout << recordA.ids[i] << "\t";
            }
            cout << " > ";
            for (int i = 0; i < tuple_size; i++) {
                cout << recordB.ids[i] << "\t";
            }
            exit(-1);
        }
        recordA = recordB;

        fileA.read((char*)buffer, tuple_size*sizeof(uint64_t));
        a = fileA.gcount();
        std::vector<uint64_t> arr;
        arr.clear(); // TODO:
        for (auto col : column_order) {
            arr.push_back(buffer[col]);
        }
        recordB = Record(arr);
        count++;
    }
    fileA.clear(); // clear badbit?
    delete[] buffer;
}