#ifndef CLDEVICEMANAGER_H__
#define CLDEVICEMANAGER_H__

#include "tgt/tgt_gl.h"
#include "tgt/manager.h"
#include "tgt/singleton.h"

#include "CL/cl.hpp"
#include "cllib/platform.h"
#include "cllib/device.h"

#include <string>


namespace cllib {

    class DeviceManager {
    public:
        DeviceManager();
        ~DeviceManager();

    private:
        std::vector<Platform*> _platforms;

        static const std::string loggerCat_;
    };

}

#endif // CLDEVICEMANAGER_H__
