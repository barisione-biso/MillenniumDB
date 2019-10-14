#ifndef FILE__RECORD_H_
#define FILE__RECORD_H_

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

class Record {
public:
    Record(std::vector<uint64_t> arr) {
        instance_count++;
        record_size = arr.size();
        ids = std::make_unique<uint64_t[]>(record_size);
        for (int i = 0; i < record_size; i++) {
            ids[i] = arr[i];
        }
    }

    Record(uint64_t e1, uint64_t e2) {
        instance_count++;
        record_size = 2;
        ids = std::make_unique<uint64_t[]>(2);
        ids[0] = e1;
        ids[1] = e2;
    }

    Record(uint64_t e1, uint64_t e2, uint64_t e3) {
        instance_count++;
        record_size = 3;
        ids = std::make_unique<uint64_t[]>(3);
        ids[0] = e1;
        ids[1] = e2;
        ids[2] = e3;
    }

    Record(uint64_t* arr, int count) { // TODO: remove constructor
        instance_count++;
        record_size = count;
        ids = std::make_unique<uint64_t[]>(count);
        for (int i = 0; i < count; i++) {
            ids[i] = arr[i];
        }
    }
    Record(const Record& copy) {
        // std::cout << "Record copy constructor\n";
        instance_count++;
        record_size = copy.record_size;
        ids = std::make_unique<uint64_t[]>(record_size);
        for (int i = 0; i < record_size; i++) {
            ids[i] = copy.ids[i];
        }
    }

    ~Record() {
        instance_count--;
    };

    std::unique_ptr<uint64_t[]> ids; // TODO: ver si usar vector del valor es más rápido
    static inline int instance_count = 0;
private:
    int record_size; // propably we don't need this
};

#endif //FILE__RECORD_H_
