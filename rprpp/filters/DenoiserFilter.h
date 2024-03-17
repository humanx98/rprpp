#pragma once

#include "Filter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>

namespace rprpp::filters {

class DenoiserFilter : public Filter {
public:
    DenoiserFilter(const std::shared_ptr<vk::helper::DeviceContext>& dctx) noexcept;
    DenoiserFilter(DenoiserFilter&&) noexcept = default;
    DenoiserFilter& operator=(DenoiserFilter&&) noexcept = default;

    DenoiserFilter(const DenoiserFilter&) = delete;
    DenoiserFilter& operator=(const DenoiserFilter&) = delete;

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setInput(Image* img) noexcept override;
    void setOutput(Image* img) noexcept override;

private:
    void validateInputsAndOutput();

    Image* m_input = nullptr;
    Image* m_output = nullptr;
    std::shared_ptr<vk::helper::DeviceContext> m_dctx;
    vk::raii::Semaphore m_finishedSemaphore;
};

}