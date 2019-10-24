#ifndef FILE__PAGE_
#define FILE__PAGE_

#include <iostream>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>

#define PAGE_SIZE 4096

class Page {
    friend class BufferManager;
    public:
        const int page_number;
        char* bytes; // protected ??
        const std::string& filename;

        void unpin() {
            pins--;
        }
    private:
        int pins;
        boost::iostreams::mapped_file mapped_file;

        Page(int page_number, const std::string& filename)
            : page_number(page_number), filename(filename)
        {
            boost::iostreams::mapped_file_params params;
            params.path = filename;
            params.length = PAGE_SIZE;
            params.mode = (std::ios_base::out | std::ios_base::in);
            params.offset = PAGE_SIZE*page_number;

            mapped_file = *new boost::iostreams::mapped_file();
            mapped_file.open(params);
            this->bytes = mapped_file.data();
            pins = 1;
        }

        Page(std::string filename)
            : page_number(0), filename(filename)
        {
            boost::iostreams::mapped_file_params params;
            params.path = filename;
            params.new_file_size = PAGE_SIZE;
            params.mode = (std::ios_base::out | std::ios_base::in);

            mapped_file = *new boost::iostreams::mapped_file();
            mapped_file.open(params);
            this->bytes = mapped_file.data();
            pins = 1;
        }

        ~Page()
        {
            std::cout << "destroying page (" << page_number << ", " << filename << ")\n";
            mapped_file.close();
        }
};

#endif //FILE__PAGE_
