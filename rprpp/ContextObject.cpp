#include "ContextObject.h"
#include "Context.h"

namespace rprpp
{

ContextObject::ContextObject(Context* parent)
: m_parent(parent),
  m_tag(parent->generateNextTag())
{
}

bool ContextObject::operator==(const ContextObject& other) const noexcept
{
	return m_tag == other.m_tag;
}

vk::helper::DeviceContext& ContextObject::deviceContext() noexcept
{
    return m_parent->deviceContext();
}

const vk::helper::DeviceContext& ContextObject::deviceContext() const noexcept
{
    return m_parent->deviceContext();
}

}