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

#ifndef RESAMPLINGDEMO_H__
#define RESAMPLINGDEMO_H__

#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"
#include "modules/base/processors/lightsourceprovider.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/preprocessing/processors/glimageresampler.h"
#include "modules/vis/processors/volumeexplorer.h"

namespace campvis {
    class CAMPVIS_MODULES_API ResamplingDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        ResamplingDemo(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~ResamplingDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "ResamplingDemo"; };

    protected:
        LightSourceProvider _lsp;
        MhdImageReader _imageReader;
        GlImageResampler _resampler;
        VolumeExplorer _ve;
    };
}

#endif // RESAMPLINGDEMO_H__
