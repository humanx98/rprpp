#pragma once

#include "rpr_helper.h"
#include "capi/rprpp.h"

class StagingBuffer 
{
public:
    StagingBuffer(RprPpContext* context, std::size_t size);
    ~StagingBuffer();

    void unmap();

    void* data() const noexcept { return m_memory; }

    StagingBuffer(const StagingBuffer&)            = delete;
    StagingBuffer& operator=(const StagingBuffer&) = delete;

private:
    RprPpContext* m_context;
    void* m_memory;
};

