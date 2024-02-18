#include "StagingBuffer.h"
#include <cassert>
#include <iostream>

StagingBuffer::StagingBuffer(RprPpContext* context, std::size_t size)
    : m_context(context)
    , m_memory(nullptr)
{
    assert(m_context);

    RprPpError status;

    status = rprppContextMapStagingBuffer(*m_context, size, &m_memory);
    RPRPP_CHECK(status);
}

void StagingBuffer::unmap()
{
    RprPpError status;

    status = rprppContextUnmapStagingBuffer(*m_context);
    RPRPP_CHECK(status);

    m_memory = nullptr;
}

StagingBuffer::~StagingBuffer()
{
    if (!m_memory)
        return;

    try {
        unmap();
    } catch (...) {
        std::cerr << "Can't unmap staging buffer";
    }
}
