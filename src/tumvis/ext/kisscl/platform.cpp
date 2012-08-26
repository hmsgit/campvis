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

#include "platform.h"

#include "tgt/logmanager.h"
#include "kisscl/device.h"
#include "core/tools/stringutils.h"

namespace kisscl {
    
    const std::string Platform::loggerCat_ = "kisscl.Platform";


// ================================================================================================

    Platform::Platform(cl_platform_id id) 
        : CLWrapper<cl_platform_id>(id)
    {
        // parse platform profile
        std::string profileString = getStringInfo(CL_PLATFORM_PROFILE);
        if (profileString == "FULL_PROFILE")
            _profile = FULL_PROFILE;
        else if (profileString == "EMBEDDED_PROFILE") {
            _profile = EMBEDDED_PROFILE;
            LWARNING("Embedded profile!");
        }
        else {
            LERROR("Unknown profile!");
        }

        // get other platform information strings
        _name = getStringInfo(CL_PLATFORM_NAME);
        _vendor = getStringInfo(CL_PLATFORM_VENDOR);
        _version = ClVersion(getStringInfo(CL_PLATFORM_VERSION));

        //explode extensions string with space as delimiter and insert them into set:
        std::vector<std::string> exploded = TUMVis::StringUtils::split(getStringInfo(CL_PLATFORM_EXTENSIONS), " ");
        _extensions.insert(exploded.begin(), exploded.end());

        //Log infos:
        LINFO("Name: " << _name);
        LINFO("Profile: " << profileString);
        LINFO("Version: " << _version);
        LINFO("Vendor: " << _vendor);
        LINFO("Extensions:");
        for(std::set<std::string>::const_iterator iter = _extensions.begin(); iter != _extensions.end(); ++iter) {
            LINFO("> " << *iter);
        }

        if (!isExtensionSupported("cl_khr_gl_sharing"))
            LWARNING("GL sharing is not supported.");

        //find devices:
        cl_uint numDevices;
        LCL_ERROR(clGetDeviceIDs(_id, CL_DEVICE_TYPE_ALL, 0, 0, &numDevices));
        LINFO("Number of devices: " << numDevices);

        cl_device_id* devices = new cl_device_id[numDevices];
        LCL_ERROR(clGetDeviceIDs(_id, CL_DEVICE_TYPE_ALL, numDevices, devices, 0));

        for(cl_uint i=0; i<numDevices; ++i) {
            _devices.push_back(new Device(this, devices[i]));
        }

        delete[] devices;
    }

    Platform::~Platform() {
        for (std::vector<Device*>::iterator it = _devices.begin(); it != _devices.end(); ++it)
            delete *it;
    }

    const std::vector<Device*>& Platform::getDevices() const {
        return _devices;
    }

    kisscl::Profile Platform::getProfile() const {
        return _profile;
    }

    std::string Platform::getName() const {
        return _name;
    }

    std::string Platform::getVendor() const {
        return _vendor;
    }

    const std::set<std::string>& Platform::getExtensions() const {
        return _extensions;
    }

    bool Platform::isExtensionSupported(const std::string& extension) const {
        return (_extensions.find(extension) != _extensions.end());
    }

    bool Platform::supportsGlSharing() const {
        return isExtensionSupported("cl_khr_gl_sharing");
    }

    const ClVersion& Platform::getVersion() const {
        return _version;
    }
    
}
