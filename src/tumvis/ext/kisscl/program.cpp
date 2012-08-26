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

#include "program.h"

#include "tgt/assert.h"
#include "tgt/filesystem.h"
#include "tgt/logmanager.h"
#include "kisscl/clruntime.h"
#include "kisscl/context.h"
#include "kisscl/device.h"
#include "kisscl/kernel.h"
#include "kisscl/platform.h"

#include <utility>

namespace kisscl {

    const std::string Program::loggerCat_ = "kisscl.Program";

    Program::Program(Context* context)
        : CLWrapper<cl_program>(0)
        , _context(context)
    {
        tgtAssert(context != 0, "Context must not be 0.");
    }

    Program::~Program() {
        clearKernels();
    }

    const std::string& Program::getBuildOptions() const {
        return _buildOptions;
    }

    void Program::setBuildOptions(const std::string& buildOptions) {
        _buildOptions = buildOptions;
    }

    const std::string& Program::getHeader() const {
        return _header;
    }

    void Program::setHeader(const std::string& header) {
        _header = header;
    }

    void Program::loadFromFile(const std::string& filename) {
        std::vector<std::string> v;
        v.push_back(filename);
        loadFromFiles(v);
    }

    void Program::loadFromFiles(const std::vector<std::string>& filenames) {
        _sources.clear();

        for(size_t i = 0; i < filenames.size(); ++i) {
            tgt::File* file = FileSys.open(filenames[i]);

            // check if file is open
            if (!file || !file->isOpen()) {
                LERROR("File not found: " << filenames[i]);
                delete file;
                break;
            }
            _sources.push_back(file->getAsString());
            file->close();
            delete file;
        }

        createProgram();
    }

    void Program::build(const std::vector<Device*>& devices /*= std::vector<Device*>()*/) {
        tgtAssert(_id != 0, "Called build() without a valid program handle. Load the program sources first!");

        clearKernels();

        // add additional include paths to build options
        std::string bo(_buildOptions);
        for (std::list<std::string>::const_iterator it = CLRtm.getPathList().begin(); it != CLRtm.getPathList().end(); ++it) {
            bo.append(" -I" + tgt::FileSystem::cleanupPath(*it));
        }

        // build program
        if (devices.empty()) {
            cl_int err = LCL_ERROR(clBuildProgram(_id, 0, 0, bo.c_str(), 0, 0));
            if (err != CL_SUCCESS) {
                for(size_t i = 0; i < _context->getDevices().size(); ++i)
                    LERROR("Build log: " << getBuildLog(_context->getDevices()[i]));
            }
        }
        else {
            cl_device_id* devIds = new cl_device_id[devices.size()];
            for(size_t i = 0; i < devices.size(); ++i)
                devIds[i] = devices[i]->getId();

            cl_int err = LCL_ERROR(clBuildProgram(_id, static_cast<cl_uint>(devices.size()), devIds, bo.c_str(), 0, 0));
            if (err != CL_SUCCESS) {
                for(size_t i = 0; i < devices.size(); ++i)
                    LERROR(getBuildLog(devices[i]));
            }
            delete[] devIds;
        }
    }

    Kernel* Program::getKernel(const std::string& name) {
        tgtAssert(_id != 0, "Called getKernel() without a valid program handle. Load the program sources first and call build()!");

        // check, whether this kernel has already been created
        std::map<std::string, Kernel*>::iterator lb = _kernels.lower_bound(name);
        if (lb == _kernels.end() || lb->first != name) {
            // no: createProgram a new kernel and insert it into cache map
            cl_int err;
            cl_kernel kernel = clCreateKernel(_id, name.c_str(), &err);
            LCL_ERROR(err);
            if ((kernel != 0) && (err == CL_SUCCESS)) {
                Kernel* toReturn = new Kernel(kernel);
                _kernels.insert(lb, std::make_pair(name, toReturn));
                return toReturn;
            }
            else
                return 0;
        }
        else {
            // yes: return the cached kernel
            return lb->second;
        }
    }

    cl_build_status Program::getBuildStatus(const Device* device) const {
        return getBuildInfo<cl_build_status>(device, CL_PROGRAM_BUILD_STATUS);
    }

    std::string Program::getBuildLog(const Device* device) const {
        return getBuildInfo<std::string>(device, CL_PROGRAM_BUILD_LOG);
    }

    //template specialization for strings:
    template<>
    std::string Program::getBuildInfo(const Device* device, cl_program_build_info info) const {
        size_t retSize;
        LCL_ERROR(clGetProgramBuildInfo(_id, device->getId(), info, 0, 0, &retSize));
        char* buffer = new char[retSize + 1];
        LCL_ERROR(clGetProgramBuildInfo(_id, device->getId(), info, retSize, buffer, 0));
        buffer[retSize] = '\0';
        std::string s(buffer);
        delete[] buffer;
        return s;
    }

    void Program::clearKernels() {
        for (std::map<std::string, Kernel*>::iterator it = _kernels.begin(); it != _kernels.end(); ++it) {
            delete it->second;
        }
        _kernels.clear();
    }

    void Program::createProgram() {
        tgtAssert(! _sources.empty(), "Cannot createProgram an OpenCL program with empty sources.");

        cl_uint numSources = _sources.size() + 1;
        const char** strings = new const char*[numSources];
        size_t* lengths = new size_t[numSources];

        strings[0] = _header.c_str();
        lengths[0] = _header.length();

        for(size_t i = 1; i < numSources; ++i)  {
            strings[i] = _sources[i-1].c_str();
            lengths[i] = _sources[i-1].length();
        }

        cl_int err;
        _id = clCreateProgramWithSource(_context->getId(), numSources, strings, lengths, &err);
        LCL_ERROR(err);

        delete[] lengths;
        delete[] strings;
    }

}
