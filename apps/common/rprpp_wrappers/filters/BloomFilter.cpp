#include "BloomFilter.h"

namespace rprpp::wrappers::filters {

BloomFilter::BloomFilter(const Context& _context)
    : Filter(_context)
{
    RprPpError status;

    RprPpFilter filter;
    status = rprppContextCreateBloomFilter(context(), &filter);
    RPRPP_CHECK(status);

    setFilter(filter);
}

void BloomFilter::setRadius(float radius)
{
    RprPpError status;

    status = rprppBloomFilterSetRadius(filter(), radius);
    RPRPP_CHECK(status);
}

void BloomFilter::setBrightnessScale(float brightnessScale)
{
    RprPpError status;

    status = rprppBloomFilterSetBrightnessScale(filter(), brightnessScale);
    RPRPP_CHECK(status);
}

void BloomFilter::setThreshold(float threshold)
{
    RprPpError status;

    status = rprppBloomFilterSetThreshold(filter(), threshold);
    RPRPP_CHECK(status);
}

}