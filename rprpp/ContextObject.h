#pragma once

#include "rprpp/vk/DeviceContext.h"
#include <boost/noncopyable.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

namespace rprpp {
class Context;
class ContextObject;

class ContextObject : public boost::noncopyable {
public:
    explicit ContextObject(Context* context);

    virtual ~ContextObject() = default;

    bool operator==(const ContextObject& other) const noexcept;

    [[nodiscard]] boost::uuids::uuid tag() const noexcept { return m_tag; }

    [[nodiscard]] Context* context() const noexcept { return m_parent; }

    [[nodiscard]] vk::helper::DeviceContext& deviceContext() noexcept;

    [[nodiscard]] const vk::helper::DeviceContext& deviceContext() const noexcept;

private:
    Context* m_parent;
    boost::uuids::uuid m_tag;
};

using ContextObjectRef = std::unique_ptr<ContextObject>;

} // namespace rprpp
