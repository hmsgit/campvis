#include "cldevicemanager.h"

#include "tgt/logmanager.h"

namespace cllib {

    const std::string DeviceManager::loggerCat_ = "cllib.DeviceManager";

    DeviceManager::DeviceManager() {
        cl_uint numPlatforms;
        LCL_ERROR(clGetPlatformIDs(0, 0, &numPlatforms));
        LINFO("Number of platformIds: " << numPlatforms);

        cl_platform_id* platformIds = new cl_platform_id[numPlatforms];
        LCL_ERROR(clGetPlatformIDs(numPlatforms, platformIds, 0));

        for(cl_uint i=0; i<numPlatforms; ++i) {
            _platforms.push_back(new Platform(platformIds[i]));
        }
        delete[] platformIds;
    }

    DeviceManager::~DeviceManager() {
        for (std::vector<Platform*>::iterator it = _platforms.begin(); it != _platforms.end(); ++it)
            delete *it;
    }

}