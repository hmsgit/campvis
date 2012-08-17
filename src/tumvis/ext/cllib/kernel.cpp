#include "kernel.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "cllib/device.h"
#include "cllib/platform.h"

namespace cllib {

    Kernel::Kernel(cl_kernel id)
        : CLWrapper<cl_kernel>(id)
    {
        tgtAssert(_id != 0, "Kernel ID must not be 0.");
    }

    cl_uint Kernel::getNumArgs() const {
        return getInfo<cl_uint>(CL_KERNEL_NUM_ARGS);
    }

}
