#ifndef BASE__STATE_H_
#define BASE__STATE_H_

#include <memory>


class State {
  State(uint64_t id, bool is_final) :
    id       (id),
    is_final (is_final) { }
  ~State() = default;

  bool transition();

  private:
    uint64_t id;
    State** neighbors;
    bool is_final;
};

#endif // BASE__STATE_H_
