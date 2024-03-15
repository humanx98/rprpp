#include "BloomFilter.h"

namespace rprpp::wrappers::filters {

BloomFilter::BloomFilter(const Context& context)
    : Filter(context)
{
    RprPpError status;

    status = rprppContextCreateBloomFilter(m_context, &m_filter);
    RPRPP_CHECK(status);
}

void BloomFilter::setRadius(float radius)
{
    RprPpError status;

    status = rprppBloomFilterSetRadius(m_filter, radius);
    RPRPP_CHECK(status);
}

void BloomFilter::setBrightnessScale(float brightnessScale)
{
    RprPpError status;

    status = rprppBloomFilterSetBrightnessScale(m_filter, brightnessScale);
    RPRPP_CHECK(status);
}

void BloomFilter::setThreshold(float threshold)
{
    RprPpError status;

    status = rprppBloomFilterSetThreshold(m_filter, threshold);
    RPRPP_CHECK(status);
}

}