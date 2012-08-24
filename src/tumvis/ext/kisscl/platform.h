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

#ifndef KISSCL_PLATFORM_H__
#define KISSCL_PLATFORM_H__

#include "kisscl/kisscl.h"

#include <set>
#include <vector>


namespace kisscl {
    class Device;
    
    /**
     * Wrapper class for an OpenCL platform.
     * Provides information about the profile, version, supported extensions, etc.
     */
    class Platform : public CLWrapper<cl_platform_id> {
    public:
        /**
         * Creates a new Platform object and initializes it by the given id.
         * \param   id  OpenCL platform id to use for initialization
         */
        Platform(cl_platform_id id);

        /**
         * Destructor, all devices will be deleted.
         */
        virtual ~Platform();

        /**
         * Gets the supported OpenCL profile.
         * \return _profile
         */
        Profile getProfile() const;

        /**
         * Gets the OpenCL platform name.
         * \return _name
         */
        std::string getName() const;

        /**
         * Gets the OpenCL platform vendor.
         * \return _vendor
         */
        std::string getVendor() const;

        /**
         * Gets the set of all supported OpenCL extensions.
         * \return _extensions
         */
        const std::set<std::string>& getExtensions() const;

        /**
         * Gets the supported OpenCL version.
         * \return _version
         */
        const ClVersion& getVersion() const;

        /**
         * Gets the list of all available OpenCL devices
         * \return _devices
         */
        const std::vector<Device*>& getDevices() const;

        /**
         * Checks, whether the OpenCL extension \a extension is supported by this platform.
         * \param   extension   OpenCL extension to check.
         * \return  _extensions.find(extension) != _extensions.end()
         */
        bool isExtensionSupported(const std::string& extension) const;

        /**
         * Checks, whether the platform supports OpenGL context sharing
         * \return  isExtensionSupported("cl_khr_gl_sharing")
         */
        bool supportsGlSharing() const;

    protected:

        Profile _profile;                   ///< supported OpenCL profile
        std::string _name;                  ///< OpenCL platform name
        std::string _vendor;                ///< OpenCL platform vendor
        std::set<std::string> _extensions;  ///< set of all supported OpenCL extensions
        ClVersion _version;                 ///< supported OpenCL version

        std::vector<Device*> _devices;      ///< List of all available OpenCL devices

        static const std::string loggerCat_;
    };


    
}

#endif // KISSCL_PLATFORM_H__
