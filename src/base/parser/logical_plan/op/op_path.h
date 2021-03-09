#ifndef BASE__OP_PATH_H_
#define BASE__OP_PATH_H_

class OpPath : public Op {
public:

    bool operator<(const OpPath& /*other*/) const {
        // TODO:
        return false;
    }
};

#endif // BASE__OP_PATH_H_