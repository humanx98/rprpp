#pragma once

#include "ContextObject.h"

#include <boost/uuid/uuid_hash.hpp>
#include <functional>

namespace std {

template <>
struct hash<rprpp::ContextObject> {
    size_t operator()(const rprpp::ContextObject& contextObject) const
    {
        return hasher(contextObject.tag());
    }

    size_t operator()(const boost::uuids::uuid uuid) const
    {
        return hasher(uuid);
    }

    boost::hash<boost::uuids::uuid> hasher;
};

} // namespace std