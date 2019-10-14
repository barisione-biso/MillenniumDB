#ifndef BASE__EDGE_H_
#define BASE__EDGE_H_

#include "base/graph/graph_element.h"
#include "base/graph/label.h"
#include "base/graph/key.h"
#include "base/graph/value/value.h"

#include <map>
#include <vector>

class Edge : public GraphElement {
public:
    Edge(u_int64_t id)
        : id(id)
    {}
    ~Edge() = default;

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

#endif //BASE__EDGE_H_
