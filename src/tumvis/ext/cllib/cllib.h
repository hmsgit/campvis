#ifndef CLLIB_H__
#define CLLIB_H__

#include "tgt/tgt_gl.h"
#include "CL/cl.hpp"

#include <string>

namespace cllib {

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


}

#endif // CLLIB_H__
