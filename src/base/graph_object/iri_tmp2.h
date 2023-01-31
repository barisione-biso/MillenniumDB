#pragma once

class IriTmp2 {
public:
    uint64_t temporal_id;

    IriTmp2(uint64_t temporal_id) : temporal_id (temporal_id) { }

    inline bool operator==(const IriTmp2& rhs) const noexcept {
        return this->temporal_id == rhs.temporal_id;
    }
};
