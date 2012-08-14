#ifndef DEVICE_H__
#define DEVICE_H__

#include "cllib/cllib.h"
#include "tgt/vector.h"

#include <set>
#include <vector>


namespace cllib {

    /**
     * Wrapper class for an OpenCL device.
     * Provides information about the profile, version, supported extensions, etc.
     */
    class Device {
    public:
        enum DeviceType {
            DEVICE_GPU = CL_DEVICE_TYPE_GPU,
            DEVICE_CPU = CL_DEVICE_TYPE_CPU
        };

        /**
         * Creates a new Device object and initializes it by the given id.
         * \param   id  OpenCL device id to use for initialization
         */
        Device(cl_device_id id);

        /**
         * Destructor, nothing happens...
         */
        ~Device();

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
        template<class T>
        T getInfo(cl_device_info info) const;

        cl_device_id _id;                   ///< OpenCL device id

        Profile _profile;                   ///< supported OpenCL profile
        std::string _name;                  ///< OpenCL device name
        std::string _vendor;                ///< OpenCL device vendor
        std::set<std::string> _extensions;  ///< set of all supported OpenCL extensions
        ClVersion _version;                 ///< supported OpenCL version

        // TODO: too lazy to implement all the getters...
        cl_device_type _deviceType;         ///< OpenCL device type
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

// ================================================================================================

    template<class T>
    T Device::getInfo(cl_device_info info) const {
        T ret;
        LCL_ERROR(clGetDeviceInfo(_id, info, sizeof(ret), &ret, 0));
        return ret;
    }

    //template specialization for strings:
    template<>
    std::string Device::getInfo(cl_device_info info) const;
    
}

#endif // DEVICE_H__