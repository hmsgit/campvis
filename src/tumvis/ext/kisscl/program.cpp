#include "program.h"

#include "tgt/assert.h"
#include "tgt/filesystem.h"
#include "tgt/logmanager.h"
#include "kisscl/context.h"
#include "kisscl/device.h"
#include "kisscl/kernel.h"
#include "kisscl/platform.h"

#include <utility>

namespace kisscl {

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

        create();
    }

    void Program::build(const std::vector<Device*>& devices /*= std::vector<Device*>()*/) {
        tgtAssert(_id != 0, "Called build() without a valid program handle. Load the program sources first!");

        clearKernels();

        if (devices.empty()) {
            LCL_ERROR(clBuildProgram(_id, 0, 0, _buildOptions.c_str(), 0, 0));
        }
        else {
            cl_device_id* devIds = new cl_device_id[devices.size()];
            for(size_t i = 0; i < devices.size(); ++i)
                devIds[i] = devices[i]->getId();
            LCL_ERROR(clBuildProgram(_id, static_cast<cl_uint>(devices.size()), devIds, _buildOptions.c_str(), 0, 0));
            delete[] devIds;
        }
    }

    Kernel* Program::getKernel(const std::string& name) {
        tgtAssert(_id != 0, "Called getKernel() without a valid program handle. Load the program sources first and call build()!");

        // check, whether this kernel has already been created
        std::map<std::string, Kernel*>::iterator lb = _kernels.lower_bound(name);
        if (lb == _kernels.end() || lb->first != name) {
            // no: create a new kernel and insert it into cache map
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
        return getBuildInfo<std::string>(device, CL_PROGRAM_BUILD_OPTIONS);
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

    void Program::create() {
        tgtAssert(! _sources.empty(), "Cannot create an OpenCL program with empty sources.");

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
