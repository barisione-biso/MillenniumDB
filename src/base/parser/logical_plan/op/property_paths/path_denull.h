#ifndef BASE__PATH_DENULL_H_
#define BASE__PATH_DENULL_H_

#include <memory>

/*
TODO: Explain class
*/

class OpPath;
class OpPathAlternatives;
class OpPathSequence;
class OpPathAtom;
class OpPathKleeneStar;
class OpPathOptional;

class PathDenull {
public:
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPath> path);

private:
    static std::unique_ptr<OpPath> accept_denull(std::unique_ptr<OpPath>);

    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathAlternatives>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathSequence>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathAtom>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathKleeneStar>);
    static std::unique_ptr<OpPath> denull(std::unique_ptr<OpPathOptional>);
};

#endif // BASE__PATH_DENULL_H_
