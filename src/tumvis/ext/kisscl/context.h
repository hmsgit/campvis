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

#ifndef KISSCL_CONTEXT_H__
#define KISSCL_CONTEXT_H__

#include "kisscl/kisscl.h"

#include <set>
#include <vector>


namespace kisscl {
    class Device;

    /**
     * OpenCL API reqiures the context callback function to be a free function - here it is...
     * \see     http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/clCreateContext.html
     * \param   errinfo         pointer to an error string
     * \param   private_info    pointer to binary data that is returned by the OpenCL implementation that can be used to log additional information helpful in debugging the error
     * \param   cb              size of \a private_info
     * \param   user_data       pointer to user supplied data - should be 0 in our case.
     */
    void CL_API_CALL clContextCallback(const char* errinfo, const void* private_info, size_t cb, void* user_data);

    /**
     * Just a wrapper around a pair of cl_context_properties.
     */
    struct ContextProperty {
        /**
         * Creates a new ContextProperty pair
         * \see     http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/clCreateContext.html
         * \param   name    Name of the OpenCL property, should/must be one of the following in: http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/enums.html#cl_context_properties
         * \param   value   Value for the OpenCL property.
         */
        ContextProperty(cl_context_properties name, cl_context_properties value)
            : _name(name)
            , _value(value)
        {}

        cl_context_properties _name;        ///< Name of the OpenCL property, should/must be one of the following in: http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/enums.html#cl_context_properties
        cl_context_properties _value;       ///< Value for the OpenCL property.
    };
    
    /**
     * Wrapper class for an OpenCL context.
     * 
     * \todo    OpenCL contexts internally maintain a reference count. We probably should use it here => implement copy constructor/assignment op.
     */
    class Context : public CLWrapper<cl_context> {
    public:
        /**
         * Creates a new OpenCL context with the given properties for the OpenCL device \a device.
         * \param   device      OpenCL device to create the context for.
         * \param   properties  List of context properties for the context to create.
         */
        Context(Device* device, const std::vector<ContextProperty>& properties = std::vector<ContextProperty>());

        /**
         * Creates a new OpenCL context with the given properties for the OpenCL devices \a devices.
         * \param   device      List of OpenCL devices to create the context for.
         * \param   properties  List of context properties for the context to create.
         */
        Context(std::vector<Device*> devices, const std::vector<ContextProperty>& properties = std::vector<ContextProperty>());


        /**
         * Returns whether the context is a valid OpenCL context
         * \return _id != 0
         */
        bool isValid() const;


        /**
         * Gets the assigned devices for this OpenCL context.
         * \return _devices
         */
        const std::vector<Device*>& getDevices() const;

        /**
         * Generates the the OpenCL context properties necessary for enabling context sharing with the current OpenGL context.
         * \return A vector of platform specific ContextProperties for OpenGL context sharing.
         */
        static std::vector<ContextProperty> generateGlSharingProperties();

    private:
        void initContext();

        std::vector<Device*> _devices;              ///< Assigned devices for this OpenCL context.
        std::vector<ContextProperty> _properties;   ///< Context properties of this context.

        static const std::string loggerCat_;
    };
    
}

#endif // KISSCL_CONTEXT_H__
