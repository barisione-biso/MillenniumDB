#ifndef BASE__VAR_H_
#define BASE__VAR_H_

#include <ostream>
#include <string>

class Var {
public:
    const std::string value;

    explicit Var(std::string value)
        : value(std::move(value)) { }

    ~Var() = default;

    inline bool operator<(const Var& rhs) const noexcept {
        return value < rhs.value;
    }

    inline bool operator<=(const Var& rhs) const noexcept {
        return value <= rhs.value;
    }

    inline bool operator>(const Var& rhs) const noexcept {
        return value > rhs.value;
    }

    inline bool operator>=(const Var& rhs) const noexcept {
        return value >= rhs.value;
    }

    inline bool operator==(const Var& rhs) const noexcept {
        return value == rhs.value;
    }

    inline bool operator!=(const Var& rhs) const noexcept {
        return value != rhs.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Var& var) {
        return os << var.value;
    }
};

#endif // BASE__VAR_H_
