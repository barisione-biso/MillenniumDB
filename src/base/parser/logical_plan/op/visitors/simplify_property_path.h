#ifndef BASE__SIMPLIFY_PROPERTY_PATH_H_
#define BASE__SIMPLIFY_PROPERTY_PATH_H_

#include <memory>

class OpPath;
class OpPathAlternatives;
class OpPathSequence;
class OpPathAtom;
class OpPathKleeneStar;
class OpPathEpsilon;

class SimplifyPropertyPath {
public:
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPath> path);

private:
    static std::unique_ptr<OpPath> accept_denull(std::unique_ptr<OpPath>);

    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathAlternatives>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathSequence>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathAtom>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathKleeneStar>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathEpsilon>);
};

#endif // BASE__SIMPLIFY_PROPERTY_PATH_H_
