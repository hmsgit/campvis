#include "device.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/platform.h"
#include "core/tools/stringutils.h"

namespace kisscl {

    const std::string Device::loggerCat_ = "kisscl.Device";
    
    Device::Device(const Platform* platform, cl_device_id id)
        : CLWrapper<cl_device_id>(id)
        , _platform(platform)
   {
        tgtAssert(_platform != 0, "Platform must not be 0.");
        tgtAssert(_id != 0, "OpenCL device id must not be 0.");

        // parse device profile
        std::string profileString = getStringInfo(CL_DEVICE_PROFILE);
        if (profileString == "FULL_PROFILE")
            _profile = FULL_PROFILE;
        else if (profileString == "EMBEDDED_PROFILE") {
            _profile = EMBEDDED_PROFILE;
            LWARNING("Embedded profile!");
        }
        else {
            LERROR("Unknown profile!");
        }

        // get other device information strings
        _name = getStringInfo(CL_DEVICE_NAME);
        LINFO("Name: " << _name);
        _vendor = getStringInfo(CL_DEVICE_VENDOR);
        _version = ClVersion(getStringInfo(CL_DEVICE_VERSION));

        //explode extensions string with space as delimiter and insert them into set:
        std::vector<std::string> exploded = TUMVis::StringUtils::split(getStringInfo(CL_DEVICE_EXTENSIONS), " ");
        _extensions.insert(exploded.begin(), exploded.end());

        _deviceType = getInfo<cl_device_type>(CL_DEVICE_TYPE);
        _maxClockFrequency = getInfo<cl_uint>(CL_DEVICE_MAX_CLOCK_FREQUENCY);
        _maxComputeUnits = getInfo<cl_uint>(CL_DEVICE_MAX_COMPUTE_UNITS);
        LCL_ERROR(clGetDeviceInfo(_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(tgt::svec3), _maxWorkItemSizes.elem, 0));
        _maxWorkGroupSize = getInfo<size_t>(CL_DEVICE_MAX_WORK_GROUP_SIZE);
        _maxMemAllocSize = getInfo<cl_ulong>(CL_DEVICE_MAX_MEM_ALLOC_SIZE);
        _globalMemSize = getInfo<cl_ulong>(CL_DEVICE_GLOBAL_MEM_SIZE);
        _localMemSize = getInfo<cl_ulong>(CL_DEVICE_LOCAL_MEM_SIZE);

        if(getInfo<cl_bool>(CL_DEVICE_IMAGE_SUPPORT)) {
            maxImageSize2D_.x = static_cast<int>(getInfo<size_t>(CL_DEVICE_IMAGE2D_MAX_WIDTH));
            maxImageSize2D_.y = static_cast<int>(getInfo<size_t>(CL_DEVICE_IMAGE2D_MAX_HEIGHT));

            maxImageSize3D_.x = static_cast<int>(getInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_WIDTH));
            maxImageSize3D_.y = static_cast<int>(getInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_HEIGHT));
            maxImageSize3D_.z = static_cast<int>(getInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_DEPTH));
        }

        //Log infos:
        LINFO("Profile: " << profileString);
        LINFO("Version: " << _version);
        LINFO("Vendor: " << _vendor);
        LINFO("Extensions:");
        for(std::set<std::string>::const_iterator iter = _extensions.begin(); iter != _extensions.end(); ++iter) {
            LINFO("> " << *iter);
        }
        if (!isExtensionSupported("cl_khr_gl_sharing"))
            LWARNING("GL sharing is not supported.");
        LINFO("Device Type: " << _deviceType);
        LINFO("Max Clock Frequency: " << _maxClockFrequency);
        LINFO("Max Compute Units: " << _maxComputeUnits);
        LINFO("Max Work Item Sizes: " << _maxWorkItemSizes);
        LINFO("Max Work Group Size: " << _maxWorkGroupSize);
        LINFO("Max Mem Alloc Size: " << _maxMemAllocSize);
        LINFO("Global Mem Size: " << _globalMemSize);
        LINFO("Local Mem Size: " << _localMemSize);
    }


    kisscl::Profile Device::getProfile() const {
        return _profile;
    }

    std::string Device::getName() const {
        return _name;
    }

    std::string Device::getVendor() const {
        return _vendor;
    }

    const std::set<std::string>& Device::getExtensions() const {
        return _extensions;
    }

    bool Device::isExtensionSupported(const std::string& extension) const {
        return (_extensions.find(extension) != _extensions.end());
    }

    bool Device::supportsGlSharing() const {
        return isExtensionSupported("cl_khr_gl_sharing");
    }

    const ClVersion& Device::getVersion() const {
        return _version;
    }

    cl_device_type Device::getDeviceType() const {
        return _deviceType;
    }

    const Platform* Device::getPlatform() const {
        return _platform;
    }

}
