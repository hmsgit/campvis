#ifndef PLATFORM_H__
#define PLATFORM_H__

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

#endif // PLATFORM_H__
