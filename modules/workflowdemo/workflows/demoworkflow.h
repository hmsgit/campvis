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

#ifndef DEMOWORKFLOW_H__
#define DEMOWORKFLOW_H__

#include "core/pipeline/abstractworkflow.h"

#include "modules/workflowdemo/pipelines/imageloading.h"
#include "modules/workflowdemo/pipelines/imagefiltering.h"
#include "modules/workflowdemo/pipelines/imagevis.h"

#include "modules/pipelinefactory.h"

namespace campvis {
namespace workflowdemo {

    class DemoWorkflow : public AbstractWorkflow {
    public:
        enum Stages {
            LOADING_STAGE,
            FILTERING_STAGE,
            VIS_STAGE
        };

        /**
         * Constructor
         */
        DemoWorkflow();

        /**
         * Virtual Destructor
         **/
        virtual ~DemoWorkflow();

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
        static const std::string getId() { return "DemoWorkflow"; };


    protected:
        void onDataContainerDataAdded(std::string key, DataHandle dh);

        void onStageChanged(int from, int to);

        ImageLoading* _loaderPipeline;
        ImageFiltering* _filterPipeline;
        ImageVis* _visPipeline;
    };

}
template class WorkflowRegistrar<workflowdemo::DemoWorkflow>;
}

#endif // DEMOWORKFLOW_H__
