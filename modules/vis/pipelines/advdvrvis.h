// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef AdvDVRVis_H__
#define AdvDVRVis_H__

#include "core/pipeline/autoevaluationpipeline.h"


#include "modules/modulesapi.h"
#include "modules/pipelinefactory.h"
#include "modules/base/processors/lightsourceprovider.h"
#include "modules/io/processors/genericimagereader.h"
#include "modules/preprocessing/processors/glimageresampler.h"
#include "modules/vis/processors/volumeexplorer.h"

namespace campvis {
    class CAMPVIS_MODULES_API AdvDVRVis : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        AdvDVRVis(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~AdvDVRVis();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "AdvDVRVis"; };


    protected:
        LightSourceProvider _lsp;
        GenericImageReader _imageReader;
        GlImageResampler _resampler;
        VolumeExplorer _ve;
    };

    // Instantiate template to register the pipelines.
    template class PipelineRegistrar<AdvDVRVis>;
}

#endif // AdvDVRVis_H__