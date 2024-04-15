#pragma once

#include "ContextObjectHash.h"
#include <boost/uuid/uuid.hpp>

namespace rprpp {
struct ContextObjectContainerHasher {
    using hash_type = std::hash<ContextObject>;
    using is_transparent = void;

    hash_type hasher;

    size_t operator()(const ContextObjectRef& obj) const
    {
        return hasher(*obj);
    }

    size_t operator()(boost::uuids::uuid uuid) const
    {
        return hasher(uuid);
    }
};

} // namespace rprpp