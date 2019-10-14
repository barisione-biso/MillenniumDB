#ifndef BASE__NODE_H_
#define BASE__NODE_H_

#include "base/graph/graph_element.h"
#include "base/graph/label.h"
#include "base/graph/key.h"
#include "base/graph/property.h"
#include "base/graph/value/value.h"

#include <map>
#include <vector>

class Node : public GraphElement {
public:
    Node(u_int64_t id)
        : id(id)
    {}
    ~Node() = default;

    void add_label(Label& label) { labels.push_back(label); }
    void add_property(Property& property) {
        properties[property.get_key()] = &property.get_value();
    }

    inline std::vector<Label>& get_labels() { return labels; }
    inline std::map<Key, Value*>& get_properties() { return properties; }
    inline bool is_node() { return true; }
    inline u_int64_t get_id() { return id; }

private:
    u_int64_t id;
    std::vector<Label> labels;
    std::map<Key, Value*> properties;
};

#endif //BASE__NODE_H_
