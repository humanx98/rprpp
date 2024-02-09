#pragma once

#include "capi/rprpp.h"
#include <stdexcept>
#include <string>

namespace rprpp {

class Error : public std::runtime_error {

public:
    RprPpError errorCode;
    Error(RprPpError err, const std::string& message)
        : std::runtime_error(message)
    {
        errorCode = err;
    }
};

class InternalError : public Error {
public:
    InternalError(const std::string& message)
        : Error(RPRPP_ERROR_INTERNAL_ERROR, "Internal error. " + message)
    {
    }
};

class InvalidParameter : public Error {
public:
    InvalidParameter(const std::string& parameterName, const std::string& message)
        : Error(RPRPP_ERROR_INVALID_PARAMETER, parameterName + "is invalid parameter. " + message)
    {
    }
};

class InvalidDevice : public Error {
public:
    InvalidDevice(uint32_t deviceId)
        : Error(RPRPP_ERROR_INVALID_DEVICE, "DeviceId " + std::to_string(deviceId) + " doesn't exist.")
    {
    }
};

class ShaderCompilationError : public Error {
public:
    ShaderCompilationError(const std::string& message)
        : Error(RPRPP_ERROR_SHADER_COMPILATION, "Shader compilation error. " + message)
    {
    }
};

}