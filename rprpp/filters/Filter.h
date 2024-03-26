#pragma once
#include "rprpp/Image.h"
#include "rprpp/vk/vk.h"

#include "rprpp/ContextObject.h"

namespace rprpp::filters {

class Filter : public ContextObject {
public:
    explicit Filter(Context* context)
        : ContextObject(context)
    {}

    virtual ~Filter() noexcept = default;

    virtual vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) = 0;
    virtual void setInput(Image* image) noexcept = 0;
    virtual void setOutput(Image* image) noexcept = 0;
};

}