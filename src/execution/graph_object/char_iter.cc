#include "char_iter.h"

#include "storage/string_manager.h"

StringExternalIter::StringExternalIter(uint64_t string_start, size_t str_len) {
    // current_page = &string_manager.get_page(id_after_strlen / StringPage::SIZE);

    // current_page_offset = (id_after_strlen) % StringPage::SIZE;
    // current = current_page->get_bytes() + (id_after_strlen % StringPage::SIZE);

    current_page_offset = string_start % StringManager::STRING_BLOCK_SIZE;
    current_block_number = string_start / StringManager::STRING_BLOCK_SIZE;
    current_block = string_manager.get_string_block(current_block_number);
    bytes_left = str_len;


    // size_t bytes_for_len;
    // bytes_left = StringManager::get_string_len(start, &bytes_for_len);
    // current = start + bytes_for_len;
}


StringExternalIter::~StringExternalIter() {
    // string_manager.unpin(*current_page);
}


char StringExternalIter::next_char() {
    if (bytes_left == 0) return '\0';

    // char res = *current;
    // current++;
    // bytes_left--;
    // return res;

    char res = current_block[current_page_offset];

    if (current_page_offset == StringManager::STRING_BLOCK_SIZE) {
        current_block_number++;
        current_block = string_manager.get_string_block(current_block_number);
        current_page_offset = 0; // TODO: supone que empieza al comienzo de la pagina, puede ser que no
    }
    current_page_offset++;
    bytes_left--;
    return res;


    // char res = current_page->get_bytes()[current_page_offset];

    // if (current_page_offset == StringPage::SIZE) {
    //     string_manager.unpin(*current_page);
    //     current_page = &string_manager.get_page(current_page->id);
    //     current_page_offset = 0;
    // }

    // current_page_offset++;
    // bytes_left--;
    // return res;
}

char IriInlineIter::next_char() {
    if (iter_prefix) {
        auto res = prefix_iter.next_char();
        if (res == '\0') {
            iter_prefix = false;
        } else {
            return res;
        }
    }
    auto res = *current;
    ++current;
    return res;
}

IriExternalIter::IriExternalIter(const std::string& prefix, uint64_t iri_id) :
    prefix_iter(StringTmpIter(prefix)),
    suffix_iter(string_manager.get_char_iter(iri_id)) { }

char IriExternalIter::next_char() {
    if (iter_prefix) {
        auto res = prefix_iter.next_char();
        if (res == '\0') {
            iter_prefix = false;
        } else {
            return res;
        }
    }
    return suffix_iter->next_char();
}

char LiteralLanguageInlineIter::next_char() {
    if (*current != '\0') {
        auto res = *current;
        ++current;
        return res;
    }
    return language_iter.next_char();
}

LiteralLanguageExternalIter::LiteralLanguageExternalIter(uint64_t literal_id, const std::string& language) :
    prefix_iter(string_manager.get_char_iter(literal_id)),
    language_iter(StringTmpIter(language)) { }

char LiteralLanguageExternalIter::next_char() {
    if (prefix_iter != nullptr) {
        auto res = prefix_iter->next_char();
        if (res == '\0') {
            prefix_iter.reset();
        } else {
            return res;
        }
    }
    return language_iter.next_char();
}

char LiteralDatatypeInlineIter::next_char() {
    if (iter_datatype) {
        auto res = datatype_iter.next_char();
        if (res == '\0') {
            iter_datatype = false;
        } else {
            return res;
        }
    }
    auto res = *current;
    ++current;
    return res;
}

LiteralDatatypeExternalIter::LiteralDatatypeExternalIter(uint64_t literal_id, const std::string& datatype) :
    literal_iter(string_manager.get_char_iter(literal_id)),
    datatype_iter(StringTmpIter(datatype)) { }

char LiteralDatatypeExternalIter::next_char() {
    if (iter_datatype) {
        auto res = datatype_iter.next_char();
        if (res == '\0') {
            iter_datatype = false;
        } else {
            return res;
        }
    }
    return literal_iter->next_char();
}