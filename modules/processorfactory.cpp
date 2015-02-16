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

#include "processorfactory.h"
//#include "gen_processorregistration.h"

#include <sstream>

namespace campvis {

    // declare one single symbol for the ProcessorFactory singleton
    tbb::atomic<ProcessorFactory*> ProcessorFactory::_singleton;

    ProcessorFactory& ProcessorFactory::getRef() {
        if (_singleton == 0) {
            std::cout << "creating ProcessorFactory...\n";
            ProcessorFactory* tmp = new ProcessorFactory();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }
    
    void ProcessorFactory::deinit() {
        delete _singleton;
        _singleton = nullptr;
    }

    std::vector<std::string> ProcessorFactory::getRegisteredProcessors() const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        std::vector<std::string> toReturn;
        toReturn.reserve(_processorMapWithIVec2Param.size()+_processorMapDefault.size());
        for (auto it = _processorMapWithIVec2Param.begin(); it != _processorMapWithIVec2Param.end(); ++it)
            toReturn.push_back(it->first);
        for (auto it = _processorMapDefault.begin(); it != _processorMapDefault.end(); ++it)
            toReturn.push_back(it->first);
        return toReturn;
    }

    AbstractProcessor* ProcessorFactory::createProcessor(const std::string& id, IVec2Property* viewPortSizeProp) const {
        tbb::spin_mutex::scoped_lock lock(_mutex);
        
        if (viewPortSizeProp != nullptr) {
            auto it = _processorMapWithIVec2Param.find(id);
            if (it == _processorMapWithIVec2Param.end())
                return nullptr;
            else
                return (it->second)(viewPortSizeProp);
        } else {
            auto it = _processorMapDefault.find(id);
            if (it == _processorMapDefault.end())
                return nullptr;
            else
                return (it->second)();
        }
    }

}
