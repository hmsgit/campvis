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

#include "pipelinefactory.h"
#include "modules/gen_pipelineregistration.h"

#include <sstream>

namespace campvis {

    // declare one single symbol for the PipelineFactory singleton
    tbb::atomic<PipelineFactory*> PipelineFactory::_singleton;

    PipelineFactory& PipelineFactory::getRef() {
        if (_singleton == 0) {
            std::cout << "creating PipelineFactory...\n";
            PipelineFactory* tmp = new PipelineFactory();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }
    
    void PipelineFactory::deinit() {
        delete _singleton;
        _singleton = nullptr;
    }


    std::vector<std::string> PipelineFactory::getRegisteredPipelines() const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        std::vector<std::string> toReturn;
        toReturn.reserve(_pipelineMap.size());
        for (auto it = _pipelineMap.begin(); it != _pipelineMap.end(); ++it)
            toReturn.push_back(it->first);
        return toReturn;
    }

    AbstractPipeline* PipelineFactory::createPipeline(const std::string& id, DataContainer* dc) const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        auto it = _pipelineMap.find(id);
        if (it == _pipelineMap.end())
            return nullptr;
        else
            return (it->second)(dc);
    }

    AbstractWorkflow* PipelineFactory::createWorkflow(const std::string& id) const {
        tbb::spin_mutex::scoped_lock lock(_mutex);

        auto it = _workflowMap.find(id);
        if (it == _workflowMap.end())
            return nullptr;
        else
            return (it->second)();
    }

}
