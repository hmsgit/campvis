// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "raycasterfactory.h"

#include <sstream>

namespace campvis {

    // declare one single symbol for the RaycasterFactory singleton
    tbb::atomic<RaycasterFactory*> RaycasterFactory::_singleton;

    RaycasterFactory& RaycasterFactory::getRef() {
        if (_singleton == 0) {
            std::cout << "creating RaycasterFactory...\n";
            RaycasterFactory* tmp = new RaycasterFactory();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }
    
    void RaycasterFactory::deinit() {
        delete _singleton;
        _singleton = nullptr;
    }


    std::vector<std::string> RaycasterFactory::getRegisteredRaycasters() const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        std::vector<std::string> toReturn;
        toReturn.reserve(_raycasterMap.size());
        for (auto it = _raycasterMap.begin(); it != _raycasterMap.end(); ++it)
            toReturn.push_back(it->first);
        return toReturn;
    }

    RaycastingProcessor* RaycasterFactory::createRaycaster(const std::string& id, IVec2Property* viewportSizeProp) const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        auto it = _raycasterMap.find(id);
        if (it == _raycasterMap.end())
            return nullptr;
        else
            return (it->second)(viewportSizeProp);
    }
}
