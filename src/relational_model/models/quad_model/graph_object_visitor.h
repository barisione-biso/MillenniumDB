#ifndef RELATIONAL_MODEL__GRAPH_OBJECT_VISITOR_H_
#define RELATIONAL_MODEL__GRAPH_OBJECT_VISITOR_H_

#include "base/graph/graph_object.h"
#include "base/ids/object_id.h"
#include "relational_model/models/quad_model/quad_model.h"

struct GraphObjectVisitor {
    QuadModel& model;

    GraphObjectVisitor(QuadModel& model);
    ~GraphObjectVisitor() = default;

    ObjectId operator()(const IdentifiableInlined& i)   const;
    ObjectId operator()(const IdentifiableExternal& i)  const;
    ObjectId operator()(const Edge& e)                  const;
    ObjectId operator()(const AnonymousNode& a)         const;
    ObjectId operator()(const StringInlined& s)         const;
    ObjectId operator()(const StringExternal& s)        const;
    ObjectId operator()(const NullGraphObject&)         const;
    ObjectId operator()(const NotFoundObject&)          const;
    ObjectId operator()(const int64_t n)                const;
    ObjectId operator()(const bool b)                   const;
    ObjectId operator()(const float f)                  const;
};

#endif // RELATIONAL_MODEL__GRAPH_OBJECT_VISITOR_H_
