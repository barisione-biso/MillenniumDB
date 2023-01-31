#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "execution/graph_object/char_iter.h"
// #include "storage/file_id.h"
//#include "storage/file_manager.h"
// #include "storage/index/hash/strings_hash/strings_hash.h"
// #include "third_party/robin_hood/robin_hood.h"

class CharIter;

class TemporalManager {
public:
    // static constexpr size_t STRING_BLOCK_SIZE = 1024*1024*64; // 64 MB

    // metadata only is in the first block
    // static constexpr size_t METADATA_SIZE = 8; // 8 bytes for end

    // necessary to be called before first usage
    static void init();
    ~TemporalManager();

    // std::unique_ptr<CharIter> get_char_iter(uint64_t id) const;

    void print_str(std::ostream& os, uint64_t id) const;
    void print_dtt(std::ostream& os, uint64_t id) const;
    void print_lan(std::ostream& os, uint64_t id) const;

    uint64_t get_str_id(const std::string&);
    uint64_t get_dtt_id(const std::string&);
    uint64_t get_lan_id(const std::string&);

    // must check the string doesn't exist before calling this method
    // uint64_t create_new(const std::string&);

    // bool str_eq(const std::string& str, uint64_t string_id) const;

    // returns the encoded string length, and writes how many bytes were read
    // static size_t get_string_len(char* ptr, size_t* bytes_for_len) {
    //     auto decode_ptr = reinterpret_cast<unsigned char*>(ptr);
    //     size_t decoded = 0;
    //     size_t shift_size = 0;
    //     *bytes_for_len = 0;
    //     while (true) {
    //         uint64_t b = *decode_ptr;
    //         decode_ptr++;
    //         (*bytes_for_len)++;

    //         if (b <= 127) {
    //             decoded |= b << shift_size;
    //             break;
    //         } else {
    //             decoded |= (b & 0x7FUL) << shift_size;
    //         }

    //         shift_size += 7;
    //     }
    //     return decoded;
    // }

    // inline char* get_string_block(size_t block_number) const { return string_blocks[block_number]; }

private:
    TemporalManager();

    // void append_new_block();

    // inline void update_last_block_offset() {
    //     auto ptr = reinterpret_cast<uint64_t*>(string_blocks[0]);
    //     *ptr = last_block_offset;
    // }

    // FileId str_file_id;

    //StringsHash str_hash;

    std::unordered_map<std::string, uint64_t> str_to_id;
    std::unordered_map<uint64_t, std::string> id_to_str;
    uint64_t next_str_id;

    std::unordered_map<std::string, uint64_t> dtt_to_id;
    std::unordered_map<uint64_t, std::string> id_to_dtt;
    uint64_t next_dtt_id;

    std::unordered_map<std::string, uint64_t> lan_to_id;
    std::unordered_map<uint64_t, std::string> id_to_lan;
    uint64_t next_lan_id;

    // uint64_t last_block_offset;

    // std::vector<char*> string_blocks;
};

extern TemporalManager& temporal_manager; // global object
