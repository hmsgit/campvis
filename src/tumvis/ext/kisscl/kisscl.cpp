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

#include "kisscl.h"

#include "tgt/logmanager.h"
#include "kisscl/stringutils.h"

namespace kisscl {
    std::string clErrorToString(cl_int err, const char* code) {
        std::string toReturn;
        switch(err) {
            case(CL_SUCCESS): toReturn = "CL_SUCCESS"; break;
            case(CL_BUILD_PROGRAM_FAILURE): toReturn = "CL_BUILD_PROGRAM_FAILURE"; break;
            case(CL_COMPILER_NOT_AVAILABLE): toReturn = "CL_COMPILER_NOT_AVAILABLE"; break;
            case(CL_DEVICE_NOT_AVAILABLE): toReturn = "CL_DEVICE_NOT_AVAILABLE"; break;
            case(CL_DEVICE_NOT_FOUND): toReturn = "CL_DEVICE_NOT_FOUND"; break;
            case(CL_IMAGE_FORMAT_MISMATCH): toReturn = "CL_IMAGE_FORMAT_MISMATCH"; break;
            case(CL_IMAGE_FORMAT_NOT_SUPPORTED): toReturn = "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
            case(CL_INVALID_ARG_INDEX): toReturn = "CL_INVALID_ARG_INDEX"; break;
            case(CL_INVALID_ARG_SIZE): toReturn = "CL_INVALID_ARG_SIZE"; break;
            case(CL_INVALID_ARG_VALUE): toReturn = "CL_INVALID_ARG_VALUE"; break;
            case(CL_INVALID_BINARY): toReturn = "CL_INVALID_BINARY"; break;
            case(CL_INVALID_BUFFER_SIZE): toReturn = "CL_INVALID_BUFFER_SIZE"; break;
            case(CL_INVALID_BUILD_OPTIONS): toReturn = "CL_INVALID_BUILD_OPTIONS"; break;
            case(CL_INVALID_COMMAND_QUEUE): toReturn = "CL_INVALID_COMMAND_QUEUE"; break;
            case(CL_INVALID_CONTEXT): toReturn = "CL_INVALID_CONTEXT"; break;
            case(CL_INVALID_DEVICE): toReturn = "CL_INVALID_DEVICE"; break;
            case(CL_INVALID_DEVICE_TYPE): toReturn = "CL_INVALID_DEVICE_TYPE"; break;
            case(CL_INVALID_EVENT): toReturn = "CL_INVALID_EVENT"; break;
            case(CL_INVALID_EVENT_WAIT_LIST): toReturn = "CL_INVALID_EVENT_WAIT_LIST"; break;
            case(CL_INVALID_GLOBAL_OFFSET): toReturn = "CL_INVALID_GLOBAL_OFFSET"; break;
            case(CL_INVALID_GLOBAL_WORK_SIZE): toReturn = "CL_INVALID_GLOBAL_WORK_SIZE"; break;
            case(CL_INVALID_GL_OBJECT): toReturn = "CL_INVALID_GL_OBJECT"; break;
            case(CL_INVALID_HOST_PTR): toReturn = "CL_INVALID_HOST_PTR"; break;
            case(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR): toReturn = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
            case(CL_INVALID_IMAGE_SIZE): toReturn = "CL_INVALID_IMAGE_SIZE"; break;
            case(CL_INVALID_KERNEL): toReturn = "CL_INVALID_KERNEL"; break;
            case(CL_INVALID_KERNEL_ARGS): toReturn = "CL_INVALID_KERNEL_ARGS"; break;
            case(CL_INVALID_KERNEL_DEFINITION): toReturn = "CL_INVALID_KERNEL_DEFINITION"; break;
            case(CL_INVALID_KERNEL_NAME): toReturn = "CL_INVALID_KERNEL_NAME"; break;
            case(CL_INVALID_MEM_OBJECT): toReturn = "CL_INVALID_MEM_OBJECT"; break;
            case(CL_INVALID_MIP_LEVEL): toReturn = "CL_INVALID_MIP_LEVEL"; break;
            case(CL_INVALID_OPERATION): toReturn = "CL_INVALID_OPERATION"; break;
            case(CL_INVALID_PLATFORM): toReturn = "CL_INVALID_PLATFORM"; break;
            case(CL_INVALID_PROGRAM): toReturn = "CL_INVALID_PROGRAM"; break;
            case(CL_INVALID_PROGRAM_EXECUTABLE): toReturn = "CL_INVALID_PROGRAM_EXECUTABLE"; break;
            case(CL_INVALID_QUEUE_PROPERTIES): toReturn = "CL_INVALID_QUEUE_PROPERTIES"; break;
            case(CL_INVALID_SAMPLER): toReturn = "CL_INVALID_SAMPLER"; break;
            case(CL_INVALID_VALUE): toReturn = "CL_INVALID_VALUE"; break;
            case(CL_INVALID_WORK_DIMENSION): toReturn = "CL_INVALID_WORK_DIMENSION"; break;
            case(CL_INVALID_WORK_GROUP_SIZE): toReturn = "CL_INVALID_WORK_GROUP_SIZE"; break;
            case(CL_INVALID_WORK_ITEM_SIZE): toReturn = "CL_INVALID_WORK_ITEM_SIZE"; break;
            case(CL_MAP_FAILURE): toReturn = "CL_MAP_FAILURE"; break;
            case(CL_MEM_COPY_OVERLAP): toReturn = "CL_MEM_COPY_OVERLAP"; break;
            case(CL_MEM_OBJECT_ALLOCATION_FAILURE): toReturn = "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
            case(CL_OUT_OF_HOST_MEMORY): toReturn = "CL_OUT_OF_HOST_MEMORY"; break;
            case(CL_OUT_OF_RESOURCES): toReturn = "CL_OUT_OF_RESOURCES"; break;
            case(CL_PROFILING_INFO_NOT_AVAILABLE): toReturn = "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
            default:
               toReturn = "unknown";
        }
        std::ostringstream tmp;
        tmp << toReturn << " (" << (int)err << ")";
        if (code != 0)
            tmp << ", caused by: " << code;
        return tmp.str();
    }

    cl_int _lCLError(cl_int err, int line, const char* file, const char* code) {
        if (err != CL_SUCCESS) {
            std::ostringstream tmp2, loggerCat;
            if (file) {
                tmp2 << " File: " << file << "@" << line;
                loggerCat << "cl-error:" << file << ':' << line;
            }
            LogMgr.log(loggerCat.str(), tgt::Error, clErrorToString(err, code), tmp2.str());
        }
        return err;
    }

// ================================================================================================

    ClVersion::ClVersion(int major /*= 0*/, int minor /*= 0*/)
        : major_(major)
        , minor_(minor)
    {
    }

    ClVersion::ClVersion(const std::string& versionString)
        : major_(-1)
        , minor_(-1)
    {
        //remove "OpenCL ":
        std::string str = versionString.substr(7);

        //ignore vendor specific part of the string:
        size_t spacePos = str.find_first_of(" ");
        if (spacePos != std::string::npos)
            str = str.substr(0, spacePos);
        else
            str = str;

        //explode version string with delimiter ".":
        std::vector<std::string> exploded = StringUtils::split(str, ".");

        // parse numbers
        if (exploded.size() < 2) {
            LERRORC("kisscl.ClVersion", "Version string too short to parse!");
        }
        else {
            major_ = StringUtils::fromString<int>(exploded[0]);
            minor_ = StringUtils::fromString<int>(exploded[1]);
        }
    }

    bool operator==(const ClVersion& x, const ClVersion& y) {
        if ((x.major_ == y.major_) && (x.minor_ == y.minor_))
            return true;
        else
            return false;
    }

    bool operator!=(const ClVersion& x, const ClVersion& y) {
        if ((x.major_ != y.major_) || (x.minor_ != y.minor_))
            return true;
        else
            return false;
    }

    bool operator<(const ClVersion& x, const ClVersion& y) {
        if (x.major_ < y.major_)
            return true;
        else if (x.major_ == y.major_)
            if (x.minor_ < y.minor_)
                return true;

        return false;
    }

    bool operator<=(const ClVersion& x, const ClVersion& y) {
        if (x.major_ < y.major_)
            return true;
        else if (x.major_ == y.major_)
            if (x.minor_ < y.minor_)
                return true;

        return false;
    }

    bool operator>(const ClVersion& x, const ClVersion& y) {
        if (x.major_ > y.major_)
            return true;
        else if (x.major_ == y.major_)
            if (x.minor_ > y.minor_)
                return true;

        return false;
    }

    bool operator>=(const ClVersion& x, const ClVersion& y) {
        if (x.major_ > y.major_)
            return true;
        else if (x.major_ == y.major_)
            if (x.minor_ > y.minor_)
                return true;

        return false;
    }

    std::ostream& operator<<(std::ostream& s, const ClVersion& v) {
        if (v.major_ == -1)
            return (s << "unknown");
        else
            return (s << v.major_ << "." << v.minor_);
    }
}

