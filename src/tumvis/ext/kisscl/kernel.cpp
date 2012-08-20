// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// This library is free software; you can redistribute it and/or modify it under the terms of the 
// GNU Lesser General Public License version 3 as published by the Free Software Foundation.
// 
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License in the file 
// "LICENSE.txt" along with this library. If not, see <http://www.gnu.org/licenses/>.
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

    void Kernel::setMemoryArgument(cl_uint index, const MemoryObject* memoryObject) {
        cl_mem mem = memoryObject->getId();
        LCL_ERROR(clSetKernelArg(_id, index, sizeof(cl_mem), &mem));
    }

}
