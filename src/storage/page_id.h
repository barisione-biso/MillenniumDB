#pragma once

#include <cstring>
#include <functional>

#include "storage/file_id.h"

struct PageId {
    FileId file_id;
    uint32_t page_number;

    PageId(FileId file_id, uint32_t page_number) :
        file_id     (file_id),
        page_number (page_number) { }

    // needed to allow std::map having PageId as key
    bool operator<(const PageId& other) const {
        if (this->file_id < other.file_id) {
            return true;
        } else if (other.file_id < this->file_id) {
            return false;
        } else {
            return this->page_number < other.page_number;
        }
    }

    // needed to allow std::unordered_map having PageId as key
    bool operator==(const PageId& other) const {
        return file_id == other.file_id && page_number == other.page_number;
    }
};

struct PageIdHasher {
    std::size_t operator()(const PageId& k) const {
        return std::hash<uint64_t>{}(static_cast<uint64_t>(k.file_id.id) | (static_cast<uint64_t>(k.page_number) << 32));
    }
};
