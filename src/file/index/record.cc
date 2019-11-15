#include <file/index/record.h>

Record Record::empty_record = Record();

Record::Record(std::vector<uint64_t> arr) {
    record_size = arr.size();
    ids = std::make_unique<uint64_t[]>(record_size);
    for (int i = 0; i < record_size; i++) {
        ids[i] = arr[i];
    }
}

Record::Record() { // only for empty Record
    record_size = 0;
    ids = nullptr;
}


Record::Record(uint64_t e1) {
    record_size = 1;
    ids = std::make_unique<uint64_t[]>(1);
    ids[0] = e1;
}

Record::Record(uint64_t e1, uint64_t e2) {
    record_size = 2;
    ids = std::make_unique<uint64_t[]>(2);
    ids[0] = e1;
    ids[1] = e2;
}

Record::Record(uint64_t e1, uint64_t e2, uint64_t e3) {
    record_size = 3;
    ids = std::make_unique<uint64_t[]>(3);
    ids[0] = e1;
    ids[1] = e2;
    ids[2] = e3;
}

Record::Record(const Record& copy) {
    record_size = copy.record_size;
    ids = std::make_unique<uint64_t[]>(record_size);
    for (int i = 0; i < record_size; i++) {
        ids[i] = copy.ids[i];
    }
}

Record& Record::get_empty_record() {
    return empty_record;
}

void Record::operator=(const Record& other) {
    // record_size = other.record_size;
    // ids = std::make_unique<uint64_t[]>(record_size);
    // TODO: check record size is the same?
    for (int i = 0; i < record_size; i++) {
        ids[i] = other.ids[i];
    }
}

bool Record::operator<=(const Record& other) {
    // TODO: check record size is the same?
    for (int i = 0; i < record_size; i++) {
        if (ids[i] < other.ids[i]) {
            return true;
        } else if (ids[i] > other.ids[i]){
            return false;
        }
    }
    return true;
}