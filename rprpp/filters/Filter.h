#pragma once
#include "rprpp/Image.h"
#include "rprpp/vk/vk.h"

namespace rprpp::filters {

class Filter {
public:
    Filter() = default;

    virtual vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) = 0;
    virtual void setInput(Image* image) noexcept = 0;
    virtual void setOutput(Image* image) noexcept = 0;

    virtual ~Filter() noexcept = default;


    Filter(const Filter&)            = delete;
    Filter& operator=(const Filter&) = delete;
};

}