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

#ifndef COLUMBIA1_H__
#define COLUMBIA1_H__

#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"
#include "modules/pipelinefactory.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/io/processors/ltfimagereader.h"
#include "modules/io/processors/vtkimagereader.h"
#include "modules/columbia/processors/geometrystrainrenderer.h"
#include "modules/columbia/processors/imageseriessplitter.h"
#include "modules/columbia/processors/strainfibertracker.h"
#include "modules/columbia/processors/strainfiberrenderer.h"
#include "modules/columbia/processors/strainraycaster.h"
#include "modules/vis/processors/geometryrenderer.h"
#include "modules/vis/processors/rendertargetcompositor.h"
#include "modules/vis/processors/sliceextractor.h"
#include "modules/vis/processors/volumerenderer.h"

namespace campvis {
    class CAMPVIS_MODULES_API Columbia1 : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        Columbia1(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~Columbia1();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "Columbia1"; };


    protected:
        DataNameProperty _boundsData;

        TrackballCameraProvider _tcp;
        LightSourceProvider _lsp;
        LtfImageReader _imageReader;
        ImageSeriesSplitter _imageSplitter;

        LtfImageReader _flowReader;
        ImageSeriesSplitter _flowSplitter;

        VtkImageReader _vtkReader;
        VolumeRenderer _vr;
        StrainRaycaster _src;
        SliceExtractor _sr;
        GeometryRenderer _gr;

        StrainFiberTracker _sft;
        StrainFiberRenderer _sfr;

        RenderTargetCompositor _compositor;

    };

    // Instantiate template to register the pipelines.
    template class PipelineRegistrar<Columbia1>;

}

#endif // COLUMBIA1_H__
