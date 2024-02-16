#include "RprPostProcessing.h"

RprPostProcessing::RprPostProcessing(uint32_t deviceId)
{
	RprPpError status;

	status = rprppCreateContext(deviceId, &m_context);
	RPRPP_CHECK(status);
}

/* RprPostProcessing::RprPostProcessing(RprPostProcessing&& other) noexcept
    : m_context(nullptr)
{
	std::swap(other.m_context, m_context);
}*/

RprPostProcessing::~RprPostProcessing()
{
	if (!m_context)
		return;

	// ignore any errors
	(void)rprppDestroyContext(m_context);
}

StagingBuffer RprPostProcessing::mapStagingBuffer(size_t size)
{
	return StagingBuffer(&m_context, size);
}

void RprPostProcessing::resize(uint32_t width, uint32_t height, RprPpImageFormat format, RprPpDx11Handle sharedDx11TextureHandle)
{
	RprPpError status;

	status = rprppContextResize(m_context, width, height, format, sharedDx11TextureHandle);
	RPRPP_CHECK(status);
}

void RprPostProcessing::getOutput(uint8_t* dst, size_t size, size_t* retSize)
{
	RprPpError status;

	status = rprppContextGetOutput(m_context, dst, size, retSize);
	RPRPP_CHECK(status);
}

void RprPostProcessing::run()
{
	RprPpError status;

	status = rprppContextRun(m_context);
	RPRPP_CHECK(status);
}

RprPpVkHandle RprPostProcessing::getVkPhysicalDevice() const 
{
	RprPpError status;
	RprPpVkHandle vkhandle;

	status = rprppContextGetVkPhysicalDevice(m_context, &vkhandle);
	RPRPP_CHECK(status);

	return vkhandle;
}

RprPpVkHandle RprPostProcessing::getVkDevice() const 
{
	RprPpError status;
	RprPpVkHandle vkhandle;

	status = rprppContextGetVkDevice(m_context, &vkhandle);
	RPRPP_CHECK(status);

	return vkhandle;
}

void RprPostProcessing::copyStagingBufferToAovColor()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovColor(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::copyStagingBufferToAovOpacity()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovOpacity(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::copyStagingBufferToAovShadowCatcher()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovShadowCatcher(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::copyStagingBufferToAovReflectionCatcher()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovReflectionCatcher(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::copyStagingBufferToAovMattePass()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovMattePass(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::copyStagingBufferToAovBackground()
{
	RprPpError status;

	status = rprppContextCopyStagingBufferToAovBackground(m_context);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setGamma(float gamma)
{
	RprPpError status;

	status = rprppContextSetGamma(m_context, gamma);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setShadowIntensity(float shadowIntensity)
{
	RprPpError status;

	status = rprppContextSetShadowIntensity(m_context, shadowIntensity);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapWhitepoint(float x, float y, float z)
{
	RprPpError status;

	status =rprppContextSetToneMapWhitepoint(m_context, x, y, z);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapVignetting(float vignetting)
{
	RprPpError status;

	status = rprppContextSetToneMapVignetting(m_context, vignetting);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapCrushBlacks(float crushBlacks)
{
	RprPpError status;

	status = rprppContextSetToneMapCrushBlacks(m_context, crushBlacks);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapBurnHighlights(float burnHighlights)
{
	RprPpError status;

	status = rprppContextSetToneMapBurnHighlights(m_context, burnHighlights);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapSaturation(float saturation)
{
	RprPpError status;

	status = rprppContextSetToneMapSaturation(m_context, saturation);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapCm2Factor(float cm2Factor)
{
	RprPpError status;

	status = rprppContextSetToneMapCm2Factor(m_context, cm2Factor);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapFilmIso(float filmIso)
{
	RprPpError status;

	status = rprppContextSetToneMapFilmIso(m_context, filmIso);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapCameraShutter(float cameraShutter)
{
	RprPpError status;

	status = rprppContextSetToneMapCameraShutter(m_context, cameraShutter);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapFNumber(float fNumber)
{
	RprPpError status;

	status = rprppContextSetToneMapFNumber(m_context, fNumber);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapFocalLength(float focalLength)
{
	RprPpError status;

	status = rprppContextSetToneMapFocalLength(m_context, focalLength);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setToneMapAperture(float aperture)
{
	RprPpError status;

	status = rprppContextSetToneMapAperture(m_context, aperture);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setBloomRadius(float radius)
{
	RprPpError status;

	status = rprppContextSetBloomRadius(m_context, radius);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setBloomBrightnessScale(float brightnessScale)
{
	RprPpError status;

	status = rprppContextSetBloomBrightnessScale(m_context, brightnessScale);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setBloomThreshold(float threshold)
{
	RprPpError status;

	status = rprppContextSetBloomThreshold(m_context, threshold);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setBloomEnabled(bool enabled)
{
	RprPpError status;

	status = rprppContextSetBloomEnabled(m_context, enabled ? RPRPP_TRUE : RPRPP_FALSE);
	RPRPP_CHECK(status);
}

void RprPostProcessing::setDenoiserEnabled(bool enabled)
{
	RprPpError status;

	status = rprppContextSetDenoiserEnabled(m_context, enabled ? RPRPP_TRUE : RPRPP_FALSE);
	RPRPP_CHECK(status);
}

