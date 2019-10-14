#ifndef BASE__LABEL_H_
#define BASE__LABEL_H_

#include <string>

class Label {
public:
    Label(std::string label_name)
        : label_name(label_name){}
    ~Label() = default;
    std::string get_label_name() const { return label_name; }

    bool operator <(const Label& rhs) const {
        return label_name < rhs.label_name;
    }

    bool operator ==(const Label& rhs) const {
        return label_name == rhs.label_name;
    }

    bool operator !=(const Label& rhs) const {
        return label_name != rhs.label_name;
    }

private:
    const std::string label_name;
};

#endif //BASE__LABEL_H_
