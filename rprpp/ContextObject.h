#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>

#include <boost/noncopyable.hpp>

#include "rprpp/vk/DeviceContext.h"

#include <functional>
#include <memory>

namespace rprpp
{
class Context;
class ContextObject;

class ContextObject : public boost::noncopyable
{
public:
    explicit ContextObject(Context* context);
	virtual ~ContextObject() = default;

    bool operator==(const ContextObject& other) const noexcept;

    boost::uuids::uuid tag() const noexcept { return m_tag; }
    Context* context() const noexcept { return m_parent; }

    vk::helper::DeviceContext& deviceContext() noexcept;
    const vk::helper::DeviceContext& deviceContext() const noexcept;

private:
    Context* m_parent;
    boost::uuids::uuid m_tag;
};

using ContextObjectRef = std::unique_ptr<ContextObject>;

} // namespace rprpp

namespace std 
{

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
