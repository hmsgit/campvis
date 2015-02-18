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

#ifndef CUDACONFIDENCEMAPSWORKFLOW_H__
#define CUDACONFIDENCEMAPSWORKFLOW_H__

#include "core/pipeline/abstractworkflow.h"

#include "modules/modulesapi.h"
#include "modules/cudaconfidencemaps/pipelines/cudaconfidencemapsdemo.h"
#include "modules/pipelinefactory.h"

namespace campvis {

    class CAMPVIS_MODULES_API CudaConfidenceMapsWorkflow : public AbstractWorkflow {
    public:
        enum Stages {
            MAIN_STAGE
        };

        /**
         * Constructor
         */
        CudaConfidenceMapsWorkflow();

        /**
         * Virtual Destructor
         **/
        virtual ~CudaConfidenceMapsWorkflow();

        /// \see AbstractWorkflow::getPipelines()
        virtual std::vector<AbstractPipeline*> getPipelines();
        /// \see AbstractWorkflow::init()
        virtual void init();
        /// \see AbstractWorkflow::deinit()
        virtual void deinit();
        /// \see AbstractWorkflow::isStageAvailable()
        virtual bool isStageAvailable(int stage) const;

        /// \see AbstractWorkflow::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "CudaConfidenceMapsWorkflow"; };

    protected:
        MetaProperty p_pipelineProperties;
        CudaConfidenceMapsDemo* _demoPipeline;
    };

    template class WorkflowRegistrar<CudaConfidenceMapsWorkflow>;
}

#endif // CUDACONFIDENCEMAPSWORKFLOW_H__
