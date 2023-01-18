#pragma once


class StringTmp2 {
public:
    uint64_t temporal_id;

    StringTmp2(uint64_t temporal_id) : temporal_id(temporal_id) { }

    inline bool operator==(const StringTmp2& rhs) const noexcept {
        return this->temporal_id == rhs.temporal_id;
    }

    inline bool operator!=(const StringTmp2& rhs) const noexcept {
        return this->temporal_id != rhs.temporal_id;
    }
};