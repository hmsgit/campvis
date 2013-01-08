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

#include "clruntime.h"

#include "tgt/logmanager.h"

namespace kisscl {

    const std::string CLRuntime::loggerCat_ = "kisscl.CLRuntime";

    CLRuntime::CLRuntime()
        : tgt::ResourceManager<Program>(false)
        , _commandQueueProperties(0)
    {
        initPlatforms();
    }

    CLRuntime::~CLRuntime() {
        // delete all command queues
         for (std::map< std::pair<Context*, Device*>, CommandQueue*>::iterator it = _commandQueues.begin(); it != _commandQueues.end(); ++it)
             delete it->second;

        // delete all platforms and with it all devices
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

    const std::vector<Device*>& CLRuntime::getCPUDevices() const {
        return _cpuDevices;
    }

    const std::vector<Device*>& CLRuntime::getGPUDevices() const {
        return _gpuDevices;
    }

    Context* CLRuntime::createGlSharingContext(const std::vector<ContextProperty>& additionalProperties /*= std::vector<ContextProperty>()*/) const {
        Context* toReturn = 0;
        std::vector<ContextProperty> properties = Context::generateGlSharingProperties();
        properties.insert(properties.end(), additionalProperties.begin(), additionalProperties.end());

        for (std::vector<Device*>::const_iterator it = _cpuDevices.begin(); it != _cpuDevices.end(); ++it) {
            toReturn = new Context(*it, properties);
            if (toReturn->isValid())
                return toReturn;

            // else 
            delete toReturn;
            toReturn = 0;
        }

        return 0;
    }

    Program* CLRuntime::loadProgram(Context* context, const std::string& filename) {
        std::vector<std::string> v;
        v.push_back(filename);
        return loadProgram(context, v);
    }

    Program* CLRuntime::loadProgram(Context* context, const std::vector<std::string>& filenames) {
        tgtAssert(context != 0, "Context must not be 0.");

        // complete paths and build unique identifier for resource manager
        std::string concatenatedFilenamens;
        std::vector<std::string> completedFilenmaes;
        completedFilenmaes.reserve(filenames.size());
        for (std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it) {
            completedFilenmaes.push_back(completePath(*it));
            concatenatedFilenamens.append(*it + "#");
        }

        Program* toReturn = new Program(context);
        toReturn->setHeader(_globalHeader);
        toReturn->loadFromFiles(completedFilenmaes);

        reg(toReturn, concatenatedFilenamens);

        return toReturn;
    }

    const std::string& CLRuntime::getGlobalHeader() const {
        return _globalHeader;
    }

    void CLRuntime::setGlobalHeader(const std::string& header) {
        _globalHeader = header;
    }


    CommandQueue* CLRuntime::getCommandQueue(Context* context, Device* device /*= 0*/) {
        tgtAssert(context != 0, "Context may not be 0.");

        if (device == 0)
            device = context->getDevices().front();

        std::pair<Context*, Device*> p = std::make_pair(context, device);

        std::map< std::pair<Context*, Device*>, CommandQueue*>::iterator lb = _commandQueues.lower_bound(p);
        if (lb == _commandQueues.end() || lb ->first != p) {
            CommandQueue* queue = new CommandQueue(context, device, _commandQueueProperties);
            _commandQueues.insert(lb, std::make_pair(p, queue));
            return queue;
        }
        else {
            return lb->second;
        }
    }

    cl_command_queue_properties CLRuntime::getCommandQueueProperties() const {
        return _commandQueueProperties;
    }

    void CLRuntime::setCommandQueueProperties(cl_command_queue_properties cqp) {
        _commandQueueProperties = cqp;
    }

}