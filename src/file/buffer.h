#ifndef FILE__BUFFER_H_
#define FILE__BUFFER_H_

#include "file/page.h"

class Buffer {
    public:
        char* get_bytes() { return bytes; }
        Buffer() {
            bytes = new char[PAGE_SIZE];
        }

        ~Buffer() {
            delete[] bytes;
        }
    private:
        char* bytes;
};

#endif //FILE__BUFFER_H_
