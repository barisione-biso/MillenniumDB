#ifndef FILE__ORDERED_FILE_H_
#define FILE__ORDERED_FILE_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class Record;

using namespace std;

class OrderedFile
{
    public:
        OrderedFile(const string& filename, uint_fast8_t record_size);
        ~OrderedFile();

        void append(Record& record);
        void order(vector<uint_fast8_t> column_order);
    private:
        std::fstream file;
        const uint_fast8_t record_size;

        void order_block(uint_fast32_t block_number, vector<uint_fast8_t>& column_order);
        bool record_greater(uint_fast32_t buffer_pos, Record& key, uint64_t* buffer, vector<uint_fast8_t>& column_order);
        void move_record_right(uint_fast32_t buffer_pos, uint64_t* buffer);
        void assign_record(Record& key, uint_fast32_t buffer_pos, uint64_t* buffer);
        void merge_blocks(uint_fast32_t block1_start_pos, uint_fast32_t block2_start_pos, uint_fast32_t merge_size);
};

#endif //FILE__ORDERED_FILE_H_
