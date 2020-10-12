#ifndef BASE__OBJECT_ID_H_
#define BASE__OBJECT_ID_H_

class ObjectId {
public:
    static constexpr uint64_t NULL_OBJECT_ID      = 0;
    static constexpr uint64_t OBJECT_ID_NOT_FOUND = UINT64_MAX;

    uint64_t id;

    explicit ObjectId(uint64_t id) :
        id(id) { }

    ObjectId() :
        id(NULL_OBJECT_ID) { } // initialized with NULL_OBJECT_ID

    ~ObjectId() = default;

    static ObjectId get_null() { return ObjectId(NULL_OBJECT_ID); }

    inline bool is_null()   const noexcept { return id == NULL_OBJECT_ID; }
    inline bool not_found() const noexcept { return id == OBJECT_ID_NOT_FOUND; }

    ObjectId& operator=(const ObjectId&) = default;
    bool operator==(const ObjectId& rhs) const noexcept { return id == rhs.id; }
    bool operator!=(const ObjectId& rhs) const noexcept { return id != rhs.id; }
};

#endif // BASE__OBJECT_ID_H_
