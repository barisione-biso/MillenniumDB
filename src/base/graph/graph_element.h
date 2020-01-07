#ifndef BASE__GRAPH_ELEMENT_H_
#define BASE__GRAPH_ELEMENT_H_

#include <map>
#include <vector>

class Key;
class Value;
class Label;
class Property;

enum class ElementType {
    NODE, EDGE
};

class GraphElement {
public:
    // virtual void add_label(Label& label) = 0;
    // virtual void add_property(Property& property) = 0;

    // virtual std::vector<Label>& get_labels() = 0;
    // virtual std::map<Key, Value*>& get_properties() = 0; // TODO: make unique or shared pointer?
    virtual bool is_node() = 0;
    virtual uint64_t get_id() = 0;
};

#endif //BASE__GRAPH_ELEMENT_H_
