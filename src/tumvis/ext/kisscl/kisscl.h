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

#ifndef KISSCL_H__
#define KISSCL_H__

#include "tgt/tgt_gl.h"
#include "CL/cl.hpp"

#include <string>

namespace kisscl {

    namespace {
        // Code for CLWrapperTraits inspired by the OpenCL C++ binding shipped with the AMD APP SDK.

        /**
         * Traits for retaining/releasing OpenCL handles.
         */
        template<typename T>
        struct CLWrapperTraits {};
        
 #if defined(CL_VERSION_1_2)
        // OpenCL 1.2 devices do have retain/release.
        template <>
        struct CLWrapperTraits<cl_device_id>
        {
            typedef cl_device_info InfoType;

            /**
             * Retain the device.
             * \param device A valid device created using createSubDevices
             * \return 
             *   CL_SUCCESS if the function executed successfully.
             *   CL_INVALID_DEVICE if device was not a valid subdevice
             *   CL_OUT_OF_RESOURCES
             *   CL_OUT_OF_HOST_MEMORY
             */
            static cl_int retain(cl_device_id device) {
                return ::clRetainDevice(device);
            }
            /**
             * Retain the device.
             * \param device A valid device created using createSubDevices
             * \return 
             *   CL_SUCCESS if the function executed successfully.
             *   CL_INVALID_DEVICE if device was not a valid subdevice
             *   CL_OUT_OF_RESOURCES
             *   CL_OUT_OF_HOST_MEMORY
             */
            static cl_int release(cl_device_id device) {
                return ::clReleaseDevice(device);
            }

            static cl_int getInfo(cl_device_id id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetDeviceInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };
#else // #if defined(CL_VERSION_1_2)
        // OpenCL 1.1 devices do not have retain/release.
        template <>
        struct CLWrapperTraits<cl_device_id> {
            typedef cl_device_info InfoType;

            // cl_device_id does not have retain().
            static cl_int retain(cl_device_id) { 
                return CL_SUCCESS;
            }
            // cl_device_id does not have release().
            static cl_int release(cl_device_id) { 
                return CL_SUCCESS;
            }

            static cl_int getInfo(cl_device_id id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetDeviceInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };
#endif // #if defined(CL_VERSION_1_2)

        template <>
        struct CLWrapperTraits<cl_platform_id> {
            typedef cl_platform_info InfoType;

            // cl_platform_id does not have retain().
            static cl_int retain(cl_platform_id) { 
                return CL_SUCCESS;
            }
            // cl_platform_id does not have release().
            static cl_int release(cl_platform_id) {
                return CL_SUCCESS;
            }

            static cl_int getInfo(cl_platform_id id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetPlatformInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_context> {
            typedef cl_context_info InfoType;

            static cl_int retain(cl_context context) {
                return ::clRetainContext(context);
            }
            static cl_int release(cl_context context) {
                return ::clReleaseContext(context);
            }

            static cl_int getInfo(cl_context id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetContextInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_command_queue> {
            typedef cl_command_queue_info InfoType;

            static cl_int retain(cl_command_queue queue) {
                return ::clRetainCommandQueue(queue);
            }
            static cl_int release(cl_command_queue queue) {
                return ::clReleaseCommandQueue(queue);
            }

            static cl_int getInfo(cl_command_queue id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetCommandQueueInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_mem> {
            typedef cl_mem_info InfoType;

            static cl_int retain(cl_mem memory) {
                return ::clRetainMemObject(memory);
            }
            static cl_int release(cl_mem memory) {
                return ::clReleaseMemObject(memory);
            }

            static cl_int getInfo(cl_mem id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetMemObjectInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_sampler> {
            typedef cl_sampler_info InfoType;

            static cl_int retain(cl_sampler sampler) {
                return ::clRetainSampler(sampler);
            }
            static cl_int release(cl_sampler sampler) {
                return ::clReleaseSampler(sampler);
            }

            static cl_int getInfo(cl_sampler id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetSamplerInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_program> {
            typedef cl_program_info InfoType;

            static cl_int retain(cl_program program) {
                return ::clRetainProgram(program);
            }
            static cl_int release(cl_program program) {

                return ::clReleaseProgram(program);}

            static cl_int getInfo(cl_program id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetProgramInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_kernel> {
            typedef cl_kernel_info InfoType;

            static cl_int retain(cl_kernel kernel) {
                return ::clRetainKernel(kernel);
            }
            static cl_int release(cl_kernel kernel) {
                return ::clReleaseKernel(kernel);
            }

            static cl_int getInfo(cl_kernel id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetKernelInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };

        template <>
        struct CLWrapperTraits<cl_event> {
            typedef cl_event_info InfoType;

            static cl_int retain(cl_event event) {
                return ::clRetainEvent(event);
            }
            static cl_int release(cl_event event) {
                return ::clReleaseEvent(event);
            }

            static cl_int getInfo(cl_event id, InfoType info, size_t paramValueSize, void* paramValue, size_t* paramValueSizeRet) {
                return clGetEventInfo(id, info, paramValueSize, paramValue, paramValueSizeRet);
            }
        };
    }

    /**
     * Helper function to transform an OpenCL error code to a string.
     * \param   err OpenCL error code.
     * \return  String representation of the error code.
     */
    std::string clErrorToString(cl_int err);

    /**
     * Helper function for debugging OpenCL errors.
     * \param   err     OpenCL error code.
     * \param   line    code line
     * \param   file    code file
     * \return  err
     */
    cl_int _lCLError(cl_int err, int line, const char* file);

    #define LCL_ERROR(err) _lCLError(err, __LINE__, __FILE__)


    /**
     * Enum for OpenCL profile
     */
    enum Profile {
        FULL_PROFILE,
        EMBEDDED_PROFILE,
        UNKNOWN
    };


    /**
     * Specifies the version of the platform OpenCL implementation.
     */
    struct ClVersion {
        /**
         * Creates a ClVersion with given major and minor version.
         * \param   major   major version
         * \param   minor   minor version
         */
        ClVersion(int major = 0, int minor = 0);

        /**
         * Creates a ClVersion from the given version string.
         * \param   versionString   The CL_PLATFORM_VERSION string.
         */
        ClVersion(const std::string& versionString);

        int major_;     ///< OpenCL major version
        int minor_;     ///< OpenCL minor version

        friend bool operator==(const ClVersion& x, const ClVersion& y);
        friend bool operator!=(const ClVersion& x, const ClVersion& y);
        friend bool operator<(const ClVersion& x, const ClVersion& y);
        friend bool operator<=(const ClVersion& x, const ClVersion& y);
        friend bool operator>(const ClVersion& x, const ClVersion& y);
        friend bool operator>=(const ClVersion& x, const ClVersion& y);
        friend std::ostream& operator<<(std::ostream& s, const ClVersion& v);
    };

    /**
     * Wrapper for OpenCL objects that maintain an internal OpenCL ID and regard the internal OpenCL reference counting.
     * \sa  CLWrapperTraits
     */
    template <typename T>
    class CLWrapper {
    public:
        /// Typedef for the type OpenCL id.
        typedef T cl_type;

        typedef typename CLWrapperTraits<T>::InfoType InfoType;

        /**
         * Default constructor for an object without id.
         */
        explicit CLWrapper() 
            : _id(0)
        { }

        /**
         * Constructor initializing the internal handle with \a id.
         * \param   id  ID of the internal OpenCL handle.
         */
        explicit CLWrapper(const cl_type& id) 
            : _id(id)
        { }

        /**
         * Destructor, releases the internal handle.
         */
        virtual ~CLWrapper() {
            if (_id != 0) 
                LCL_ERROR(release());
        }

        /**
         * Copy constructor, regards the internal reference counting.
         * \param   rhs Source object
         */
        CLWrapper(const CLWrapper<cl_type>& rhs)
        {
            _id = rhs._id;
            if (_id != 0)
                LCL_ERROR(retain()); 
        }

        /**
         * Assignment operator, regards the internal reference counting.
         * \param   rhs Source object
         * \return  *this
         */
        CLWrapper<cl_type>& operator=(const CLWrapper<cl_type>& rhs)
        {
            if (_id != rhs._id) {
                if (_id != 0)
                    LCL_ERROR(release()); 

                _id = rhs._id;
                if (_id != 0)
                    LCL_ERROR(retain()); 
            }
            return *this;
        }

        /**
         * Gets the handle to internal OpenCl object.
         * \return  _id
         */
        cl_type getId() const {
            return _id;
        }


    protected:
        /**
         * Internally retains this object by using CLWrapperTraits<cl_type>.
         * \param   id  Object to retain.
         * \return  The Error code of the retain function call.
         */
        cl_int retain() const {
            return CLWrapperTraits<cl_type>::retain(_id);
        }

        /**
         * Internally releases this object by using CLWrapperTraits<cl_type>.
         * \return  The Error code of the release function call.
         */
        cl_int release() const {
            return CLWrapperTraits<cl_type>::release(_id);
        }

        /**
         * Get the information about this object from OpenCL.
         * \note    Do \b use it with std::string, use getStringInfo() instead.
         * \param   info    The information to query.
         */
        template<class R>
        R getInfo(InfoType info) const {
            R ret;
            LCL_ERROR(CLWrapperTraits<cl_type>::getInfo(_id, info, sizeof(ret), &ret, 0));
            return ret;
        }

        /**
         * Get the information about this object from OpenCL as string representation.
         * \param   info    The information to query.
         */
        std::string getStringInfo(InfoType info) const {
            size_t retSize;
            LCL_ERROR(CLWrapperTraits<T>::getInfo(_id, info, 0, 0, &retSize));
            char* buffer = new char[retSize + 1];
            LCL_ERROR(CLWrapperTraits<T>::getInfo(_id, info, retSize, buffer, 0));
            std::string ret(buffer);
            delete[] buffer;
            return ret;
        }

        cl_type _id;        ///< Handle to internal OpenCL object.
    };

}

#endif // KISSCL_H__
