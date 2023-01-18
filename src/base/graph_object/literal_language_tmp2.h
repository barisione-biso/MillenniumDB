#pragma once

class LiteralLanguageTmp2 {
public:
    uint64_t temporal_id;

    LiteralLanguageTmp2(uint64_t temporal_id) : temporal_id(temporal_id) { }

    inline bool operator==(const LiteralLanguageTmp2& rhs) const noexcept {
        return this->temporal_id == rhs.temporal_id;
    }

    inline bool operator!=(const LiteralLanguageTmp2& rhs) const noexcept {
        return this->temporal_id != rhs.temporal_id;
    }
};
