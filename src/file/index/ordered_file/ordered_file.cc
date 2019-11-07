#include "file/index/ordered_file/ordered_file.h"

#include <memory>
#include "file/index/record.h"

#ifndef ORDER_BLOCK_SIZE
    #define ORDER_BLOCK_SIZE 4096
#endif

OrderedFile::OrderedFile(const string& filename, uint_fast8_t record_size)
    : record_size(record_size)
{
    file = fstream(filename, fstream::in|fstream::out|fstream::binary|fstream::app);
}


OrderedFile::~OrderedFile() {
    file.close();
}

void OrderedFile::append(Record& record) {
    file.seekg(0, file.end);

    for (uint_fast8_t i = 0; i < record_size; i++) {
        file.write((const char *)&record.ids[i], sizeof(uint64_t));
    }
}

void OrderedFile::order(vector<uint_fast8_t> column_order) {
    file.seekg(0, file.end);

    uint64_t size_in_bytes = file.tellg();
    uint_fast32_t size_in_blocks = size_in_bytes / ORDER_BLOCK_SIZE;
    if (size_in_bytes%ORDER_BLOCK_SIZE != 0) {
        size_in_blocks++;
    }

    for (uint_fast32_t i = 0; i < size_in_blocks; i++) {
        order_block(i, column_order);
    }

    // MERGE
    uint_fast32_t merge_size = 1;
    while (merge_size < size_in_blocks) {

        for (uint_fast32_t i = 0; i < size_in_blocks; i+= merge_size*2) {
            merge_blocks(i, i+merge_size, merge_size);
        }
        merge_size *= 2;
    }
}

void OrderedFile::order_block(uint_fast32_t block_number, vector<uint_fast8_t>& column_order) {
    file.seekg(0, file.end);
    uint_fast32_t tuples_in_block;
    if ((uint_fast32_t)file.tellg() >= ORDER_BLOCK_SIZE*(block_number+1)*sizeof(uint64_t)) { // this block is full
        tuples_in_block = ORDER_BLOCK_SIZE;
    }
    else { // this block is not full
        tuples_in_block = file.tellg() % (ORDER_BLOCK_SIZE*sizeof(uint64_t));
    }

    uint64_t* buffer = new uint64_t[tuples_in_block*record_size];

    file.seekg(block_number*ORDER_BLOCK_SIZE);
    file.readsome((char*)buffer,  tuples_in_block*sizeof(uint64_t));

    // insertion sort
    for (uint_fast32_t i = 1; i < tuples_in_block; i++) {
        vector<uint64_t> vector_key = vector<uint64_t>(record_size);
        for (uint_fast8_t n = 0; n < record_size; n++) {
            vector_key.push_back(buffer[i*record_size + n]);
        }
        Record key(std::move(vector_key));

        uint_fast32_t j = i - 1;
        while (record_greater(j, key, buffer, column_order))
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
    delete[](buffer);
}

// returns true if Key is greater than the record in the buffer at buffer_pos
bool OrderedFile::record_greater(uint_fast32_t buffer_pos, Record& key, uint64_t* buffer, vector<uint_fast8_t>& column_order) {
    for (auto column : column_order) {
        if (key.ids[column] > buffer[buffer_pos*record_size + column]) {
            return true;
        }
        else if (key.ids[column] < buffer[buffer_pos*record_size + column]) {
            return false;
        }
    }
    return false;// they are equal
}

void OrderedFile::move_record_right(uint_fast32_t buffer_pos, uint64_t* buffer) {
    for (uint_fast8_t i = 8; i < record_size; i++) {
        buffer[(buffer_pos+1)*record_size + i] = buffer[buffer_pos*record_size + i];
    }
}

void OrderedFile::assign_record(Record& key, uint_fast32_t buffer_pos, uint64_t* buffer) {
    for (uint_fast8_t i = 8; i < record_size; i++) {
        buffer[buffer_pos*record_size + i] = key.ids[i];
    }
}

void OrderedFile::merge_blocks(uint_fast32_t block1_start_pos, uint_fast32_t block2_start_pos, uint_fast32_t merge_size) {
    // TODO: do merge
}