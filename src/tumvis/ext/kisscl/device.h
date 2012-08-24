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
//  * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific prior written
//    permission.
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

#ifndef KISSCL_DEVICE_H__
#define KISSCL_DEVICE_H__

#include "kisscl/kisscl.h"
#include "tgt/vector.h"

#include <set>
#include <vector>


namespace kisscl {
    class Platform;

    /**
     * Wrapper class for an OpenCL device.
     * Provides information about the profile, version, supported extensions, etc.
     */
    class Device : public CLWrapper<cl_device_id> {
    public:
        enum DeviceType {
            DEVICE_GPU = CL_DEVICE_TYPE_GPU,
            DEVICE_CPU = CL_DEVICE_TYPE_CPU
        };

        /**
         * Creates a new Device object and initializes it by the given id.
         * \param   id  OpenCL device id to use for initialization
         */
        Device(const Platform* platform, cl_device_id id);

        /**
         * Gets the parent platform of the device.
         * \return _platform
         */
        const Platform* getPlatform() const;

        /**
         * Gets the supported OpenCL profile.
         * \return _profile
         */
        Profile getProfile() const;

        /**
         * Gets the OpenCL device name.
         * \return _name
         */
        std::string getName() const;

        /**
         * Gets the OpenCL device vendor.
         * \return _vendor
         */
        std::string getVendor() const;

        /**
         * Gets the supported OpenCL version.
         * \return _version
         */
        const ClVersion& getVersion() const;

        /**
         * Gets the OpenCL device type.
         * \return _deviceType
         */
        cl_device_type getDeviceType() const;
        /**
         * Gets the set of all supported OpenCL extensions.
         * \return _extensions
         */
        const std::set<std::string>& getExtensions() const;

        /**
         * Checks, whether the OpenCL extension \a extension is supported by this device.
         * \param   extension   OpenCL extension to check.
         * \return  _extensions.find(extension) != _extensions.end()
         */
        bool isExtensionSupported(const std::string& extension) const;

        /**
         * Checks, whether the device supports OpenGL context sharing
         * \return  isExtensionSupported("cl_khr_gl_sharing")
         */
        bool supportsGlSharing() const;

    protected:
        const Platform* _platform;          ///< Parent platform of the device.

        Profile _profile;                   ///< supported OpenCL profile
        std::string _name;                  ///< OpenCL device name
        std::string _vendor;                ///< OpenCL device vendor
        std::set<std::string> _extensions;  ///< set of all supported OpenCL extensions
        ClVersion _version;                 ///< supported OpenCL version
        cl_device_type _deviceType;         ///< OpenCL device type

        // TODO: too lazy to implement all the getters...
        cl_uint _maxClockFrequency;         ///< Maximum configured clock frequency of the device in MHz.
        cl_uint _maxComputeUnits;           ///< The number of parallel compute cores on the OpenCL device.
        tgt::svec3 _maxWorkItemSizes;       ///< Maximum number of work-items that can be specified in each dimension of the work-group to clEnqueueNDRangeKernel.
        size_t _maxWorkGroupSize;           ///< Maximum number of work-items in a work-group executing a kernel using the data parallel execution model.
        cl_ulong _maxMemAllocSize;          ///< Max size of memory object allocation in bytes.
        cl_ulong _globalMemSize;            ///< Size of global device memory in bytes.
        cl_ulong _localMemSize;             ///< Size of local memory arena in bytes.
        tgt::ivec2 maxImageSize2D_;         ///< Max size of 2D images in pixels.
        tgt::ivec3 maxImageSize3D_;         ///< Max size of 3D images in pixels.


        static const std::string loggerCat_;
    };
   
}

#endif // KISSCL_DEVICE_H__
