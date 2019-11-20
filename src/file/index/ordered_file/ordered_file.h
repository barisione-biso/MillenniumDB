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
        const uint_fast8_t tuple_size;

        OrderedFile(const string& filename, uint_fast8_t tuple_size);
        ~OrderedFile();

        void append_record(const Record& record);
        void order(vector<uint_fast8_t> column_order);

        void print();
        void check_order(vector<uint_fast8_t> column_order);

        void begin_iter();
        bool has_more_tuples();
        uint_fast32_t next_tuples(uint64_t* output, uint_fast32_t max_tuples);

    private:
        std::fstream file;
        std::fstream tmp_file;
        const string filename;
        const uint_fast8_t bytes_per_tuple;
        const uint_fast32_t block_size_in_bytes;
        uint_fast32_t current_output_pos;
        uint64_t* output_buffer;
        uint64_t* big_buffer;
        uint64_t** buffer;
        long filesize;

        void create_run(uint64_t* buffer, uint_fast32_t block_number, vector<uint_fast8_t>& column_order);
        bool record_less_than(uint_fast32_t buffer_pos, uint64_t* key, uint64_t* buffer, vector<uint_fast8_t>& column_order);
        void move_record_right(uint_fast32_t buffer_pos, uint64_t* buffer);
        void assign_record(uint64_t* key, uint_fast32_t buffer_pos, uint64_t* buffer);
        // void merge_blocks(uint_fast32_t block1_start_pos, uint_fast32_t block2_start_pos, uint_fast32_t merge_size);
};

#endif //FILE__ORDERED_FILE_H_
