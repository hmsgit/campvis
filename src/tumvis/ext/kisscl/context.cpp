// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//  * The names of its contributors may not be used to endorse or promote products derived from
//    this software without specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "context.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/device.h"
#include "kisscl/platform.h"

#ifdef WIN32
#include <windows.h>
#endif

#if !(defined(WIN32) || defined(__APPLE__))
#include <GL/glx.h>
#define CL_GLX_DISPLAY_KHR 0x200A
#endif

namespace kisscl {

    void CL_API_CALL clContextCallback(const char* errinfo, const void* private_info, size_t cb, void* user_data) {
        tgtAssert (user_data == 0, "user_data expected to be 0 - check what happened!");
        LINFOC("kisscl.Context.clContextCallback", std::string(errinfo));
    }

    const std::string Context::loggerCat_ = "kisscl.Context";


    Context::Context(Device* device, const std::vector<ContextProperty>& properties /*= std::vector<ContextProperty>()*/)
        : CLWrapper<cl_context>(0)
        , _properties(properties)
    {
        _devices.push_back(device);
        initContext();
    }

    Context::Context(std::vector<Device*> devices, const std::vector<ContextProperty>& properties /*= std::vector<ContextProperty>()*/)
        : CLWrapper<cl_context>(0)
        , _devices(devices)
        , _properties(properties)
    {
        tgtAssert(devices.size() > 0, "Must provide at least one OpenCL device.");
        initContext();
    }


    void Context::initContext() {
        cl_int err;

        // prepare device list for API call
        cl_uint numDevices = _devices.size();
        cl_device_id* deviceIds = new cl_device_id[numDevices];
        cl_platform_id platformId = _devices[0]->getPlatform()->getId();
        for (size_t i = 0; i < numDevices; ++i) {
            deviceIds[i] = _devices[i]->getId();
            if (_devices[i]->getPlatform()->getId() != platformId) {
                LWARNING("The platforms of the given OpenCL devices differ. This results in undefined behavior!");
            }
        }

        // prepare properties list for API call
        int numProps = static_cast<int>((2 * _properties.size()) + 1); // +1 for terminating 0
        cl_context_properties* props = new cl_context_properties[numProps];
        for(size_t i = 0; i < _properties.size(); ++i) {
            props[2*i] = _properties[i]._name;
            props[(2*i) + 1] = _properties[i]._value;
        }
        props[numProps - 1] = 0;

        // API call
        _id = clCreateContext(props, numDevices, deviceIds, &clContextCallback, 0, &err);
        LCL_ERROR(err);

        // clean up
        delete[] deviceIds;
        delete[] props;
    }

    std::vector<ContextProperty> Context::generateGlSharingProperties() {
        std::vector<ContextProperty> toReturn;
#ifdef WIN32
        //Windows:
        toReturn.push_back(ContextProperty(CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext()));
        toReturn.push_back(ContextProperty(CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC()));
#elif defined(linux) || defined(__linux__)
        //Linux:
        toReturn.push_back(ContextProperty(CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext()));
        toReturn.push_back(ContextProperty(CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay()));
#endif
        return toReturn;
    }

    bool Context::isValid() const {
        return _id != 0;
    }

    const std::vector<Device*>& Context::getDevices() const {
        return _devices;
    }

}
