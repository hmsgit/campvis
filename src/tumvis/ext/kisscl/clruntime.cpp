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

#include "clruntime.h"

#include "tgt/logmanager.h"

namespace kisscl {

    const std::string CLRuntime::loggerCat_ = "kisscl.CLRuntime";

    CLRuntime::CLRuntime() {
        initPlatforms();
    }

    CLRuntime::~CLRuntime() {
        for (std::vector<Platform*>::iterator it = _platforms.begin(); it != _platforms.end(); ++it)
            delete *it;
    }

    void CLRuntime::initPlatforms() {
        // gather OpenCL platforms and create wrapper objects
        cl_uint numPlatforms;
        LCL_ERROR(clGetPlatformIDs(0, 0, &numPlatforms));
        LINFO("Number of platformIds: " << numPlatforms);

        cl_platform_id* platformIds = new cl_platform_id[numPlatforms];
        LCL_ERROR(clGetPlatformIDs(numPlatforms, platformIds, 0));

        for(cl_uint i=0; i<numPlatforms; ++i) {
            _platforms.push_back(new Platform(platformIds[i]));
        }
        delete[] platformIds;

        // sort devices into shortcut lists
        for (std::vector<Platform*>::const_iterator pit = _platforms.begin(); pit != _platforms.end(); ++pit) {
            for (std::vector<Device*>::const_iterator dit = (*pit)->getDevices().begin(); dit != (*pit)->getDevices().end(); ++dit) {
                // check for CPU device type
                if ((*dit)->getDeviceType() & CL_DEVICE_TYPE_CPU)
                    _cpuDevices.push_back(*dit);

                // check for GPU device type
                if ((*dit)->getDeviceType() & CL_DEVICE_TYPE_GPU)
                    _gpuDevices.push_back(*dit);
            }
        }
    }

    const std::vector<Device*> CLRuntime::getCPUDevices() const {
        return _cpuDevices;
    }

    const std::vector<Device*> CLRuntime::getGPUDevices() const {
        return _gpuDevices;
    }

    Context* CLRuntime::createGlSharingContext(const std::vector<ContextProperty>& additionalProperties /*= std::vector<ContextProperty>()*/) const {
        Context* toReturn = 0;
        std::vector<ContextProperty> properties = Context::generateGlSharingProperties();
        properties.insert(properties.end(), additionalProperties.begin(), additionalProperties.end());

        for (std::vector<Device*>::const_iterator it = _gpuDevices.begin(); it != _gpuDevices.end(); ++it) {
            toReturn = new Context(*it, properties);
            if (toReturn->isValid())
                return toReturn;

            // else 
            delete toReturn;
            toReturn = 0;
        }

        return 0;
    }

}