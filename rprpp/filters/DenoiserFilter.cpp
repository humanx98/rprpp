#include "DenoiserFilter.h"
#include "rprpp/Error.h"
#include "rprpp/rprpp.h"

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

DenoiserFilter::DenoiserFilter(Context* context) 
    : Filter(context)
    , m_finishedSemaphore(deviceContext().device.createSemaphore({}))
{
}

void DenoiserFilter::validateInputsAndOutput()
{
    if (m_input == nullptr) {
        throw InvalidParameter("input", "cannot be null");
    }

    if (m_output == nullptr) {
        throw InvalidParameter("output", "cannot be null");
    }

    if (m_input->description().width != m_output->description().width
        || m_input->description().height != m_output->description().height) {
        throw InvalidParameter("output and input", "output and input should have the same image description");
    }
}

vk::Semaphore DenoiserFilter::run(std::optional<vk::Semaphore> waitSemaphore)
{
    validateInputsAndOutput();

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
    vk::SubmitInfo submitInfo;
    if (waitSemaphore.has_value()) {
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setWaitSemaphores(waitSemaphore.value());
    }

    submitInfo.setSignalSemaphores(*m_finishedSemaphore);
    deviceContext().queue.submit(submitInfo);
    return *m_finishedSemaphore;
}

void DenoiserFilter::setInput(Image* image)
{
    m_input = image;
}

void DenoiserFilter::setOutput(Image* image)
{
    m_output = image;
}

}