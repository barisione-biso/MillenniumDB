#ifndef BASE__PROPERTY_H_
#define BASE__PROPERTY_H_

#include <string>

class Key;
class Value;

class Property {
public:
    Property(Key& key, Value& value)
        : key(key), value(value){}
    ~Property() = default;

    inline Key& get_key() const { return key; }
    inline Value& get_value() const { return value; }

private:
    Key& key;
    Value& value;
};

#endif //BASE__PROPERTY_H_
