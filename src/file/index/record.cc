#include <file/index/record.h>

using namespace std;

Record Record::empty_record = Record();

Record::Record(vector<uint64_t> arr) {
    ids = arr;
}


Record::Record() { } // only for empty Record
Record::~Record() = default;


Record::Record(uint64_t e1) {
    ids = std::vector<uint64_t>(1);
    ids[0] = e1;
}


Record::Record(uint64_t e1, uint64_t e2) {
    ids = std::vector<uint64_t>(2);
    ids[0] = e1;
    ids[1] = e2;
}


Record::Record(uint64_t e1, uint64_t e2, uint64_t e3) {
    ids = std::vector<uint64_t>(3);
    ids[0] = e1;
    ids[1] = e2;
    ids[2] = e3;
}


Record::Record(const Record& copy) {
    ids = copy.ids;
}


void Record::operator=(const Record& other) {
    ids = other.ids;
}


bool Record::operator<=(const Record& other) {
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i] < other.ids[i]) {
            return true;
        } else if (ids[i] > other.ids[i]){
            return false;
        }
    }
    return true;
}


bool Record::operator<(const Record& other) {
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i] < other.ids[i]) {
            return true;
        } else if (ids[i] > other.ids[i]){
            return false;
        }
    }
    return false;
}
