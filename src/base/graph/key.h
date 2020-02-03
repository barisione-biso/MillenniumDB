#ifndef BASE__KEY_H_
#define BASE__KEY_H_

#include <string>

class Key {
private:
    const std::string key_name;

public:
    Key(std::string key_name)
        : key_name(key_name) { }
    ~Key() = default;

    std::string get_key_name() const {
        return key_name;
    }

    bool operator <(const Key& rhs) const {
        return key_name < rhs.key_name;
    }

    bool operator ==(const Key& rhs) const {
        return key_name == rhs.key_name;
    }

    bool operator !=(const Key& rhs) const {
        return key_name != rhs.key_name;
    }
};

#endif //BASE__KEY_H_
