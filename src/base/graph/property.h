#ifndef BASE__PROPERTY_H_
#define BASE__PROPERTY_H_

#include <string>

class Key;
class Value;

class Property {
public:
    Property(const Key& key, const Value& value)
        : key(key), value(value){}
    ~Property() = default;

    const Key& get_key() const { return key; }
    const Value& get_value() const { return value; }

private:
    const Key& key;
    const Value& value;
};

#endif //BASE__PROPERTY_H_
