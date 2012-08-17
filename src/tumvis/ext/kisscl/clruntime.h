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

#ifndef KISSCL_CLRUNTIME_H__
#define KISSCL_CLRUNTIME_H__

#include "tgt/tgt_gl.h"
#include "tgt/manager.h"
#include "tgt/singleton.h"

#include "kisscl/kisscl.h"
#include "kisscl/context.h"
#include "kisscl/device.h"
#include "kisscl/platform.h"

#include <string>


namespace kisscl {
    /**
     * Singleton class for managing the OpenCL runtime.
     * Gathers all available OpenCL platforms/devices and offers methods to create OpenCL contexts on them.
     */
    class CLRuntime {
    public:
        CLRuntime();
        ~CLRuntime();

        /**
         * Tries to create an OpenCL context, which is shared with the current OpenGL context.
         * Therefore, it consecutively tries each GPU device until context creation was successful.
         * \param   additionalProperties    Additional properties for the context to create. Do 
         * \return 
         */
        Context* createGlSharingContext(const std::vector<ContextProperty>& additionalProperties = std::vector<ContextProperty>()) const;

        /**
         * Gets the list of all available OpenCL CPU devices.
         * \return _cpuDevices
         */
        const std::vector<Device*> getCPUDevices() const;

        /**
         * Gets the list of all available OpenCL GPU devices.
         * \return _gpuDevices
         */
        const std::vector<Device*> getGPUDevices() const;

    private:
        /**
         * Gathers and inits all available platforms and their devices.
         */
        void initPlatforms();

        std::vector<Platform*> _platforms;      ///< List of all OpenCL platforms and their devices

        std::vector<Device*> _cpuDevices;       ///< List of all OpenCL CPU devices (just a shortcut to the corresponding devices in _platforms)
        std::vector<Device*> _gpuDevices;       ///< List of all OpenCL GPU devices (just a shortcut to the corresponding devices in _platforms)


        static const std::string loggerCat_;
    };

}

#endif // KISSCL_CLRUNTIME_H__
