// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// This library is free software; you can redistribute it and/or modify it under the terms of the 
// GNU Lesser General Public License version 3 as published by the Free Software Foundation.
// 
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License in the file 
// "LICENSE.txt" along with this library. If not, see <http://www.gnu.org/licenses/>.
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
