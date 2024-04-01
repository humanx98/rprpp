#pragma once

#include "ContextObject.h"
#include "ContextObjectContainerHasher.h"
#include "ContextObjectContainerEqual.h"
#include <boost/uuid/uuid_generators.hpp>
#include <unordered_set>

namespace rprpp
{

class ContextObjectContainer : public boost::noncopyable
{
public:
    template <class T, class... Params>
    auto emplace(Params&&... params)
    {
        return m_objects.emplace(std::make_unique<T>(std::forward<Params>(params)...));
    }

    template <class T, class... Params>
    T* emplaceCastReturn(Params&&... params)
    {
        const auto iter = emplace<T>(std::forward<Params>(params)...);
        return static_cast<T*>(iter.first->get());
    }

    auto erase(boost::uuids::uuid uuid)
    {
        auto iter = m_objects.find(uuid);
        return m_objects.erase(iter);
    }

    [[warning("Performance penalty")]]
    void erase(ContextObject* address)
    {
        for (auto iter = m_objects.begin(); iter != m_objects.end(); ++iter) {
            const ContextObject* ptr = iter->get();
            if (!ptr)
                continue;

            if (ptr == address) {
                m_objects.erase(iter);
                break;
            }
        }
    }

    boost::uuids::uuid generateNextTag();

private:
    boost::uuids::random_generator m_tagGenerator;
    std::unordered_set<ContextObjectRef, ContextObjectContainerHasher, ContextObjectContainerEqual> m_objects;
};

} // namespace rprpp