#pragma once

#include "Filter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>

#include <OpenImageDenoise/oidn.hpp>

namespace rprpp::filters {

class DenoiserFilter : public Filter {
public:
    explicit DenoiserFilter(Context* context, oidn::DeviceRef& device);

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setInput(Image* img) override;
    void setOutput(Image* img) override;

private:
    void initialize();
    void validateInputsAndOutput();

    bool m_dirty;

    oidn::DeviceRef m_device;

    oidn::BufferRef m_colorBuffer;
    oidn::BufferRef m_albedoBuffer;
    oidn::BufferRef m_normalBuffer;
    oidn::BufferRef m_outputBuffer;

    oidn::FilterRef m_filter;

    Image* m_input = nullptr;
    Image* m_output = nullptr;
    vk::raii::Semaphore m_finishedSemaphore;
};

}