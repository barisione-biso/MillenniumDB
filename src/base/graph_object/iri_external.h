#pragma once

class IriExternal {
public:
    uint64_t external_id;

    IriExternal(uint64_t external_id) : external_id(external_id) { }

    // std::string to_string() const { }

    // void print(std::ostream& os) const { }

    inline bool operator==(const IriExternal& rhs) const noexcept {
        return this->external_id == rhs.external_id;
    }

    inline bool operator!=(const IriExternal& rhs) const noexcept {
        return this->external_id != rhs.external_id;
    }

    // inline bool operator<=(const IriExternal& rhs) const noexcept { }

    // inline bool operator>=(const IriExternal& rhs) const noexcept { }

    // inline bool operator<(const IriExternal& rhs) const noexcept { }

    // inline bool operator>(const IriExternal& rhs) const noexcept { }
};
