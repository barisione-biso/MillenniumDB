#ifndef BASE__CONDITION_H_
#define BASE__CONDITION_H_

#include <memory>

// Abstract class
class Condition {
public:
    /* TODO: pass binding to eval?*/
    virtual bool eval() = 0;
    virtual bool is_conjunction() = 0;
    virtual void add_to_conjunction(std::unique_ptr<Condition>) = 0;
};

#endif //BASE__CONDITION_H_
