#pragma once


class LiteralDatatypeTmp2 {
public:
    uint64_t temporal_id;

    LiteralDatatypeTmp2(uint64_t temporal_id) : temporal_id(temporal_id) { }

    inline bool operator==(const LiteralDatatypeTmp2& rhs) const noexcept {
        return this->temporal_id == rhs.temporal_id;
    }

    inline bool operator!=(const LiteralDatatypeTmp2& rhs) const noexcept {
        return this->temporal_id != rhs.temporal_id;
    }
};
