// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "cudaconfidencemapsworkflow.h"


namespace campvis {

    CudaConfidenceMapsWorkflow::CudaConfidenceMapsWorkflow() 
        : AbstractWorkflow("CudaConfidenceMapsWorkflow")
        , p_pipelineProperties("PipelineProperties", "Main Settings")
        , _demoPipeline(nullptr)
    {
        _demoPipeline = new CudaConfidenceMapsDemo(_dataContainer);
        p_pipelineProperties.addPropertyCollection(*_demoPipeline);


        typedef std::vector< std::pair<AbstractPipeline*, bool> > VisVec;
        typedef std::vector<AbstractProperty*> PropVec;

        VisVec vvl;
        vvl.push_back(std::make_pair(_demoPipeline, true));
        PropVec pvl;
        pvl.push_back(&p_pipelineProperties);
        addStage(MAIN_STAGE, "Real-time Confidence Maps", vvl, pvl);
    }

    CudaConfidenceMapsWorkflow::~CudaConfidenceMapsWorkflow() {

    }

    std::vector<AbstractPipeline*> CudaConfidenceMapsWorkflow::getPipelines() {
        std::vector<AbstractPipeline*> toReturn;
        toReturn.push_back(_demoPipeline);
        return toReturn;
    }

    void CudaConfidenceMapsWorkflow::init() {
        // nothing special to do here apart from calling base method
        AbstractWorkflow::init();
    }

    void CudaConfidenceMapsWorkflow::deinit() {

    }

    bool CudaConfidenceMapsWorkflow::isStageAvailable(int stage) const {
        switch (stage) {
            case MAIN_STAGE:
                return true;
            default:
                return false;
        }
    }

}
