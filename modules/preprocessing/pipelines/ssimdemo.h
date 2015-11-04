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

#ifndef SSIMDEMO_H__
#define SSIMDEMO_H__

#include "core/tools/glreduction.h"
#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/preprocessing/processors/glgaussianfilter.h"
#include "modules/preprocessing/processors/glstructuralsimilarity.h"
#include "modules/vis/processors/usfanrenderer.h"

namespace campvis {
    class CAMPVIS_MODULES_API SsimDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a SsimDemo pipeline.
         * \param   dataContainer   Reference to the DataContainer containing local working set of data
         *                          for this pipeline, must be valid the whole lifetime of this pipeline.
         */
        explicit SsimDemo(DataContainer& dataContainer);

        /**
         * Virtual Destructor
         **/
        virtual ~SsimDemo();

        virtual void init() override;
        virtual void deinit() override;


        static const std::string getId() { return "SsimDemo"; };

    protected:
        void onProcessorValidated(AbstractProcessor* p);

        void executeBatchProcess();

        DevilImageReader _imageReader1;
        DevilImageReader _imageReader2;
        GlStructuralSimilarity _ssim;
        UsFanRenderer _fanRenderer;

        GlReduction* _sumReduction;
        GlReduction* _minReduction;
        bool _currentlyBatchProcessing;

        StringProperty p_sourcePath1;
        StringProperty p_sourcePath2;

        IVec2Property p_range;                          ///< Range for image iteration
        ButtonProperty p_execute;                       ///< Button to start the batch process
    };
}

#endif // SSIMDEMO_H__
