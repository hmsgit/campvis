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
//  * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific prior written
//    permission.
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

#ifndef KISSCL_CLRUNTIME_H__
#define KISSCL_CLRUNTIME_H__

#include "tgt/tgt_gl.h"
#include "tgt/manager.h"
#include "tgt/singleton.h"

#include "kisscl/kisscl.h"
#include "kisscl/context.h"
#include "kisscl/commandqueue.h"
#include "kisscl/device.h"
#include "kisscl/platform.h"
#include "kisscl/program.h"

#include <map>
#include <string>
#include <utility>


namespace kisscl {
    /**
     * Singleton class for managing the OpenCL runtime.
     * Gathers all available OpenCL platforms/devices, offers methods to create OpenCL contexts on them and
     * manages the command queue for each context-device pair.Furthermore, CLRuntime acts as resource manager
     * for OpenCL programs. Caching of them is currently disabled.
     * 
     * \see tgt::Singleton, tgt::ResourceManager
     */
    class CLRuntime : public tgt::Singleton<CLRuntime>, public tgt::ResourceManager<Program> {
        friend class tgt::Singleton<CLRuntime>;

    public:
        /**
         * Destructor, cleaning up all platforms and devices.
         */
        ~CLRuntime();

        /**
         * Tries to create an OpenCL context, which is shared with the current OpenGL context.
         * Therefore, it consecutively tries each GPU device until context creation was successful.
         * \param   additionalProperties    Additional properties for the context to create. Do 
         * \return 
         */
        Context* createGlSharingContext(const std::vector<ContextProperty>& additionalProperties = std::vector<ContextProperty>()) const;

        /**
         * Gets the list of all available OpenCL CPU devices.
         * \return _cpuDevices
         */
        const std::vector<Device*>& getCPUDevices() const;

        /**
         * Gets the list of all available OpenCL GPU devices.
         * \return _gpuDevices
         */
        const std::vector<Device*>& getGPUDevices() const;

        /**
         * Creates a new OpenCL program from the given context from the file specified by \a filename.
         * \param   context     OpenCL context the program shall live in.
         * \param   filename    Filename of the source file.
         * \return 
         */
        Program* loadProgram(Context* context, const std::string& filename);

        /**
         * Creates a new OpenCL program from the given context from the files specified by \a filename.
         * \param   context     OpenCL context the program shall live in.
         * \param   filenames   List of the filenames of the source files.
         * \return 
         */
        Program* loadProgram(Context* context, const std::vector<std::string>& filenames);

        /**
         * Gets the global header for OpenCL programs.
         * \return _header
         */
        const std::string& getGlobalHeader() const;

        /**
         * Sets the global header for OpenCL programs to \a header.
         * \param header The new global header for OpenCL programs.
         */
        void setGlobalHeader(const std::string& header);

        /**
         * Gets the Command queue properties bitfield for all newly created command queues.
         * \return _commandQueueProperties
         */
        cl_command_queue_properties getCommandQueueProperties() const;

        /**
         * Sets the Command queue properties bitfield for all newly created command queues.
         * \param   cqp     The new Command queue properties bitfield for all newly created command queues.
         */
        void setCommandQueueProperties(cl_command_queue_properties cqp);

        /**
         * Gets the command queue for the given OpenCL context-device pair.
         * If no such command queue has yet been requested, a new one will be created.
         * \param   context     OpenCL context to create the command queue for. 
         * \param   device      OpenCL device to create the command queue for, if 0 the first device of the given context will be taken (default).
         * \param   properties  Command queue properties bitfield.
         * \return  The command queue for the given context-device pair.
         */
        CommandQueue* getCommandQueue(Context* context, Device* device = 0);

    private:
        /**
         * Gathers and inits all available platforms and their devices.
         */
        void initPlatforms();

        std::vector<Platform*> _platforms;      ///< List of all OpenCL platforms and their devices

        std::vector<Device*> _cpuDevices;       ///< List of all OpenCL CPU devices (just a shortcut to the corresponding devices in _platforms)
        std::vector<Device*> _gpuDevices;       ///< List of all OpenCL GPU devices (just a shortcut to the corresponding devices in _platforms)

        std::map< std::pair<Context*, Device*>, CommandQueue*> _commandQueues;
        cl_command_queue_properties _commandQueueProperties;        ///< Command queue properties bitfield for all newly created command queues.

        std::string _globalHeader;              ///< The global header for OpenCL programs.

        /**
         * Private constructor for singleton pattern.
         */
        CLRuntime();

        static const std::string loggerCat_;
    };

#define CLRtm tgt::Singleton<kisscl::CLRuntime>::getRef()

}

#endif // KISSCL_CLRUNTIME_H__
