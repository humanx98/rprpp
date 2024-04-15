#pragma once

#include "DenoiserFilter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>

#include <OpenImageDenoise/oidn.hpp>

namespace rprpp::filters {

class DenoiserGpuFilter : public DenoiserFilter {
public:
    explicit DenoiserGpuFilter(Context* context, oidn::DeviceRef& device);
    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;

private:
    void initialize();
    static std::unique_ptr<Buffer> createStagingBufferFor(Image* image);
};

}