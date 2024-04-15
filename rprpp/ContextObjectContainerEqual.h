#include "ContextObject.h"
#include <boost/uuid/uuid.hpp>

namespace rprpp {
struct ContextObjectContainerEqual {
    using is_transparent = void;

    bool operator()(const ContextObjectRef& obj1, const ContextObjectRef& obj2) const
    {
        return *obj1 == *obj2;
    }

    bool operator()(boost::uuids::uuid uuid, const ContextObjectRef& obj2) const
    {
        return uuid == obj2->tag();
    }
};

} // namespace rprpp
