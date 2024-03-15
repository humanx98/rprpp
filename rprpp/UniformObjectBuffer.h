#pragma once

#include "Buffer.h"

namespace rprpp {

template <class T>
class UniformObjectBuffer {
public:
    UniformObjectBuffer(Buffer&& buffer) noexcept;
    UniformObjectBuffer(UniformObjectBuffer&&) noexcept = default;
    UniformObjectBuffer& operator=(UniformObjectBuffer&&) noexcept = default;

    static UniformObjectBuffer create(const vk::helper::DeviceContext& dctx);

    T& data() noexcept;
    const T& data() const noexcept;
    size_t size() const noexcept;
    vk::Buffer buffer() const noexcept;
    bool dirty() const noexcept;
    void update();
    void markDirty() noexcept;

    UniformObjectBuffer(const Buffer&) = delete;
    UniformObjectBuffer& operator=(const Buffer&) = delete;

private:
    bool m_dirty = true;
    T m_data;
    Buffer m_buffer;
};

template <class T>
UniformObjectBuffer<T>::UniformObjectBuffer(Buffer&& buffer) noexcept
    : m_buffer(std::move(buffer))
{
}

template <class T>
UniformObjectBuffer<T> UniformObjectBuffer<T>::create(const vk::helper::DeviceContext& dctx)
{
    Buffer buffer = Buffer::create(dctx,
        sizeof(T),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    return UniformObjectBuffer(std::move(buffer));
}

template <class T>
T& UniformObjectBuffer<T>::data() noexcept
{
    return m_data;
}

template <class T>
const T& UniformObjectBuffer<T>::data() const noexcept
{
    return m_data;
}

template <class T>
bool UniformObjectBuffer<T>::dirty() const noexcept
{
    return m_dirty;
}

template <class T>
size_t UniformObjectBuffer<T>::size() const noexcept
{
    return m_buffer.size();
}

template <class T>
vk::Buffer UniformObjectBuffer<T>::buffer() const noexcept
{
    return m_buffer.get();
}

template <class T>
void UniformObjectBuffer<T>::update()
{
    void* data = m_buffer.map(sizeof(T));
    std::memcpy(data, &m_data, sizeof(T));
    m_buffer.unmap();
    m_dirty = false;
}

template <class T>
void UniformObjectBuffer<T>::markDirty() noexcept
{
    m_dirty = true;
}

}