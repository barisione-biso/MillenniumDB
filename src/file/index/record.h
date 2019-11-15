#ifndef FILE__RECORD_H_
#define FILE__RECORD_H_

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

class Record {
public:
    std::unique_ptr<uint64_t[]> ids; // TODO: ver si usar vector del valor es más rápido
    static Record& get_empty_record();

    Record(std::vector<uint64_t> arr);
    Record(uint64_t e1);
    Record(uint64_t e1, uint64_t e2);
    Record(uint64_t e1, uint64_t e2, uint64_t e3);

    Record(const Record& copy);
    ~Record() = default;

    void operator=(const Record& other);
    bool operator<(const Record& other);
    bool operator<=(const Record& other);


private:
    Record();
    int record_size; // we don't need this if vector is used
    static Record empty_record;
};

#endif //FILE__RECORD_H_
