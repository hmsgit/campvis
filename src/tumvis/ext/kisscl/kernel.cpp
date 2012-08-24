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

#include "kernel.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/device.h"
#include "kisscl/platform.h"

namespace kisscl {

    Kernel::Kernel(cl_kernel id)
        : CLWrapper<cl_kernel>(id)
    {
        tgtAssert(_id != 0, "Kernel ID must not be 0.");
    }

    cl_uint Kernel::getNumArgs() const {
        return getInfo<cl_uint>(CL_KERNEL_NUM_ARGS);
    }

//     void Kernel::setMemoryArgument(cl_uint index, const MemoryObject* memoryObject) {
//         cl_mem mem = memoryObject->getId();
//         LCL_ERROR(clSetKernelArg(_id, index, sizeof(cl_mem), &mem));
//     }

    template<>
    void Kernel::setArgument(cl_uint index, const Buffer& data) {
        cl_mem mem = data.getId();
        LCL_ERROR(clSetKernelArg(_id, index, sizeof(cl_mem), &mem));
    }
    template<>
    void Kernel::setArgument(cl_uint index, const Image& data) {
        cl_mem mem = data.getId();
        LCL_ERROR(clSetKernelArg(_id, index, sizeof(cl_mem), &mem));
    }

    template<>
    void Kernel::setArgument(cl_uint index, const GLTexture& data) {
        cl_mem mem = data.getId();
        LCL_ERROR(clSetKernelArg(_id, index, sizeof(cl_mem), &mem));
    }

}
