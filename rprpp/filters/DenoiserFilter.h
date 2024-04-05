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
    explicit DenoiserFilter(Context* context);

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setInput(Image* img) override;
    void setOutput(Image* img) override;

private:
    void validateInputsAndOutput();

    Image* m_input = nullptr;
    Image* m_output = nullptr;
    vk::raii::Semaphore m_finishedSemaphore;
};

}