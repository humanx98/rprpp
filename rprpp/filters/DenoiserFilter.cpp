#include "DenoiserFilter.h"
#include "rprpp/Error.h"
#include <cassert>

#include <boost/log/trivial.hpp>

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

DenoiserFilter::DenoiserFilter(Context* context, oidn::DeviceRef& device)
    : Filter(context),
      m_dirty(true),
      m_device(device),
      m_finishedSemaphore(deviceContext().device.createSemaphore({}))
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
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::run";

    validateInputsAndOutput();

    if (m_dirty) {
        initialize();
        m_dirty = false;
    }

    // Fill the input image buffers
    float* colorPtr = (float*)m_colorBuffer.getData();
    // copy image here

    //m_filter.executeAsync();
    m_filter.execute();

    // Check for errors
    const char* errorMessage;
    if (m_device.getError(errorMessage) != oidn::Error::None) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
        throw std::runtime_error(errorMessage);
    }

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

void DenoiserFilter::initialize()
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::initialize";
    if (!m_input)
        throw std::runtime_error("DenoiserFilter: can't initialize, not set input");

    if (!m_output)
        throw std::runtime_error("DenoiserFilter: can't initalize, not set output");

    const unsigned int width = m_input->description().width;
    const unsigned int height = m_input->description().height;

    m_colorBuffer = m_device.newBuffer(width * height * 3 * sizeof(float));
    m_albedoBuffer = m_device.newBuffer(width * height * 3 * sizeof(float));
    m_normalBuffer = m_device.newBuffer(width * height * 3 * sizeof(float));
    m_outputBuffer = m_device.newBuffer(width * height * 3 * sizeof(float));

    m_filter = m_device.newFilter("RT"); // generic ray tracing filter

    m_filter.setImage("color",  m_colorBuffer,  oidn::Format::Float3, width, height); // beauty
    m_filter.setImage("albedo", m_albedoBuffer, oidn::Format::Float3, width, height); // auxiliary
    m_filter.setImage("normal", m_normalBuffer, oidn::Format::Float3, width, height); // auxiliary
    m_filter.setImage("output", m_colorBuffer,  oidn::Format::Float3, width, height); // denoised beauty
    //m_filter.set("hdr", true); // beauty image is HDR
    m_filter.commit();

    BOOST_LOG_TRIVIAL(debug) << "denoiser filter created";
}

void DenoiserFilter::setInput(Image* image)
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::setInput";

    assert(image);

    m_input = image;
    m_dirty = true;
}

void DenoiserFilter::setOutput(Image* image)
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::setOutput";

    m_output = image;
    m_dirty = true;
}

}