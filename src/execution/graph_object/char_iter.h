#pragma once

#include <memory>
#include <string>

#include "storage/file_id.h"

// class StringPage;

class CharIter {
public:
    virtual ~CharIter() = default;

    // Once '\0' is returned, calling again next_char() is undefined behaviour.
    virtual char next_char() = 0;
};


class StringTmpIter : public CharIter {
    std::string::const_iterator current;
    std::string::const_iterator end;

public:
    StringTmpIter(const std::string& str) :
        current (str.begin()),
        end     (str.end()) { }

    char next_char() override {
        if (current == end) {
            return '\0';
        } else {
            auto res = *current;
            ++current;
            return res;
        }
    }
};


class StringInlineIter : public CharIter {
    int64_t encoded_value;
    char* current;

public:
    StringInlineIter(int64_t _encoded_value) :
        encoded_value (_encoded_value)
    {
        current = reinterpret_cast<char*>(&encoded_value);
    }

    char next_char() override {
        auto res = *current;
        ++current;
        return res;
    }
};


class StringExternalIter : public CharIter {
    const char* current_block;

    uint_fast32_t bytes_left;
    uint_fast32_t current_page_offset;
    uint_fast32_t current_block_number;

public:
    StringExternalIter(uint64_t string_start, size_t str_len);
    // StringExternalIter(char* ptr);
    ~StringExternalIter();

    char next_char() override;
};

class IriTmpIter : public CharIter {
    std::string::const_iterator current;
    std::string::const_iterator end;

public:
    IriTmpIter(const std::string& str) :
        current (str.begin()),
        end     (str.end()) { }

    char next_char() override;
};

class IriInlineIter : public CharIter {
    StringTmpIter prefix_iter;
    char* current;
    bool iter_prefix = true;

public:
    IriInlineIter(const std::string& prefix, const char suffix[7]) :
        prefix_iter(prefix)
    {
        current = reinterpret_cast<char*>(&suffix);
    }

    char next_char() override;
};

class IriExternalIter : public CharIter {
    StringTmpIter prefix_iter;
    std::unique_ptr<CharIter> suffix_iter;
    bool iter_prefix = true;
public:
    IriExternalIter(const std::string& prefix, uint64_t iri_id);

    char next_char() override;
};

class LiteralWithSuffixInlineIter : public CharIter {
    StringTmpIter suffix_iter;
    char* current;
    bool iter_prefix = true;

public:
    LiteralWithSuffixInlineIter(const char prefix[6], const std::string& suffix) :
        suffix_iter(suffix)
    {
        current = reinterpret_cast<char*>(&prefix);
    }

    char next_char() override;
};

class LiteralWithSuffixExternalIter : public CharIter {
    std::unique_ptr<CharIter> prefix_iter;
    StringTmpIter suffix_iter;
    bool iter_prefix = true;
public:
    LiteralWithSuffixExternalIter(uint64_t literal_id, const std::string& suffix);

    char next_char() override;
};