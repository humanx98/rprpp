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

    void setInput(Image* img) override;
    void setOutput(Image* img) override;

    void setAovAlbedo(Image* img);
    void setAovNormal(Image* img);

protected:
    void validateInputsAndOutput();
    void copyImageToBuffer(vk::helper::CommandBuffer& commandBuffer, Image* image, Buffer* buffer);
    void copyBufferToImage(vk::helper::CommandBuffer& commandBuffer, Buffer* buffer, Image* image);

    bool m_dirty = true;
    oidn::DeviceRef m_device;
    oidn::FilterRef m_filter;
    oidn::BufferRef m_colorBuffer;
    oidn::BufferRef m_albedoBuffer;
    oidn::BufferRef m_normalBuffer;
    std::unique_ptr<Buffer> m_stagingColorBuffer;
    std::unique_ptr<Buffer> m_stagingAlbedoBuffer;
    std::unique_ptr<Buffer> m_stagingNormalBuffer;

    Image* m_input = nullptr;
    Image* m_albedo = nullptr;
    Image* m_normal = nullptr;
    Image* m_output = nullptr;
    vk::raii::Semaphore m_finishedSemaphore;
    vk::helper::CommandBuffer m_copyInputsCommands;
    vk::helper::CommandBuffer m_copyOutputCommand;
};

}