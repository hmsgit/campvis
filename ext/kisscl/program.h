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

#ifndef KISSCL_PROGRAM_H__
#define KISSCL_PROGRAM_H__

#include "kisscl/kisscl.h"

#include <map>
#include <string>
#include <vector>

namespace kisscl {
    class Context;
    class Device;
    class Kernel;

    /**
     * Wrapper for an OpenCL program.
     */
    class Program : public CLWrapper<cl_program> {
    public:
        /**
         * Creates a new OpenCL program for the given context.
         * \param   context OpenCL context the program shall live in.
         */
        Program(Context* context);

        /**
         * Destructor, cleans up all kernels.
         * \return 
         */
        virtual ~Program();

        /**
         * Gets the build options for the OpenCL program.
         * \return _buildOptions
         */
        const std::string& getBuildOptions() const;

        /**
         * Sets The build options for the OpenCL program to \a buildOptions.
         * \param buildOptions The new build options for the OpenCL program.
         */
        void setBuildOptions(const std::string& buildOptions);

        /**
         * Gets the global header for the OpenCL program.
         * \return _header
         */
        const std::string& getHeader() const;

        /**
         * Sets the global header for the OpenCL program to \a header.
         * \param header The new global header for the OpenCL program.
         */
        void setHeader(const std::string& header);


        /**
         * Loads the program from a file.
         * \param   filename    Filename of the source file.
         */
        void loadFromFile(const std::string& filename);

        /**
         * Loads the program from multiple files.
         * \param   filenames   List of the filenames of the source files.
         */
        void loadFromFiles(const std::vector<std::string>& filenames);

        /**
         * Builds (compiles and links) this OpenCL program for the given devices.
         * If \a devices is empty, the program will be built for all deviecs associated with this 
         * program (its context).
         * \param   devices List of OpenCL devices to build the program for, may be empty (default).
         */
        void build(const std::vector<Device*>& devices = std::vector<Device*>());

        /**
         * Creates and returns the OpenCL kernel with the given name.
         * \note    Beware: The returned kernel will be invalidated/deleted on calling build().
         * \note    The returned kernel is owned by this program. Do \b not even think of deleting
         *          it yourself. Program will take care of that at a suitable time.
         * \param   name    The function name in the program (must be declared with the __kernel qualifier)
         * \return  The corresponding kernel object or 0 if kernel creation was not successful.
         */
        Kernel* getKernel(const std::string& name);

        /**
         * Gets the build status of this program on the given device.
         * \param   device  Device to query build status for.
         * \return  Returns the build, compile or link status, whichever was performed last on program for 
         *          device. This can be one of the following:
         *          CL_BUILD_NONE, CL_BUILD_ERROR, CL_BUILD_SUCCESS, CL_BUILD_IN_PROGRESS.
         */
        cl_build_status getBuildStatus(const Device* device) const;

        /**
         * Gets the build log of this program on the given device.
         * \param   device  Device to query build log for.
         * \return  The build log of this program on \a device.
         */
        std::string getBuildLog(const Device* device) const;



    private:
        /**
         * Deletes all built kernels.
         */
        void clearKernels();

        /**
         * Creates the actual OpenCL program.
         * \note    _sources should contain valid sources when calling this method.
         */
        void createProgram();

        //TODO: get binaries
        template<class T>
        T getBuildInfo(const Device* device, cl_program_build_info info) const;

        Context* _context;
        
        std::string _buildOptions;                  ///< The build options for the OpenCL program.
        std::string _header;                        ///< The global header for the OpenCL program.
        std::vector<std::string> _sources;          ///< List of all sources to build.
        std::map<std::string, Kernel*> _kernels;    ///< List of all kernels built for this program.

        static const std::string loggerCat_;

    };

// = Template definition ==========================================================================

    template<class T>
    T Program::getBuildInfo(const Device* device, cl_program_build_info info) const {
        T ret;
        LCL_ERROR(clGetProgramBuildInfo(_id, device->getId(), info, sizeof(ret), &ret, 0));
        return ret;
    }


    //template specialization for strings:
    template<>
    std::string Program::getBuildInfo(const Device* device, cl_program_build_info info) const;

}

#endif // KISSCL_PROGRAM_H__