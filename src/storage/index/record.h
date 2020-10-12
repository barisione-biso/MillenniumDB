#ifndef STORAGE__RECORD_H_
#define STORAGE__RECORD_H_

#include <array>
#include <cstdint>
#include <ostream>

template <std::size_t N>
class Record {
public:
    std::array<uint64_t, N> ids;

    Record(std::array<uint64_t, N> ids) noexcept :
        ids(move(ids)) { }

    Record(const Record<N>& copy) :
        ids(copy.ids) { }

    ~Record() = default;

    void operator=(const Record<N>& other)  {
        ids = other.ids;
    }

    bool operator<(const Record<N>& other) const {
        for (size_t i = 0; i < N; i++) {
            if (ids[i] < other.ids[i]) {
                return true;
            } else if (ids[i] > other.ids[i]){
                return false;
            }
        }
        return false;
    }

    bool operator<=(const Record<N>& other) const {
        for (size_t i = 0; i < N; i++) {
            if (ids[i] < other.ids[i]) {
                return true;
            } else if (ids[i] > other.ids[i]){
                return false;
            }
        }
        return true;
    }

    // template <std::size_t M>
    // friend std::ostream& operator<< (std::ostream& os, const Record<N>& record);
    // template <std::size_t M>
    friend std::ostream& operator<<(std::ostream& os, const Record<N>& record) {
        std::string res = '(' + std::to_string(record.ids[0]); // Suposing record size > 0
        for (size_t i = 1; i < N; i++) {
            res += ", " + std::to_string(record.ids[i]);
        }
        res += ')';
        return os << res;
    }
};

class RecordFactory {
public:
    static Record<1> get(uint64_t a1) {
        return Record<1>(std::array<uint64_t, 1> { a1 });
    }

    static Record<2> get(uint64_t a1, uint64_t a2) {
        return Record<2>(std::array<uint64_t, 2> { a1, a2 });
    }

    static Record<3> get(uint64_t a1, uint64_t a2, uint64_t a3) {
        return Record<3>(std::array<uint64_t, 3> { a1, a2, a3 });
    }

    static Record<4> get(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        return Record<4>(std::array<uint64_t, 4> { a1, a2, a3, a4 });
    }

};

#endif // STORAGE__RECORD_H_
