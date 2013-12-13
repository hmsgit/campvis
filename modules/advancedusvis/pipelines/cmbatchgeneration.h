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

#ifndef CMBATCHGENERATION_H__
#define CMBATCHGENERATION_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/io/processors/csvdimagereader.h"
#include "modules/advancedusvis/processors/advancedusfusion.h"
#include "modules/preprocessing/processors/gradientvolumegenerator.h"
#include "modules/preprocessing/processors/lhhistogram.h"
#include "modules/itk/processors/itkimagefilter.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/simpleraycaster.h"
#include "modules/vis/processors/quadview.h"
#include "modules/randomwalk/processors/confidencemapgenerator.h"

#include "core/properties/buttonproperty.h"
#include "core/properties/genericproperty.h"

namespace campvis {
    class CmBatchGeneration : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline. 
         */
        CmBatchGeneration(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~CmBatchGeneration();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "CmBatchGeneration"; };

        /**
         * Execute this pipeline.
         **/
        void execute();
        
    protected:
        /**
         * Slot getting called when one of the observed processors got invalidated.
         * Overwrites the default behaviour to do nothing.
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);

        void executePass(int path);

        void save(int path, const std::string& basePath);

        DevilImageReader _usReader;
        ConfidenceMapGenerator _confidenceGenerator;
        ItkImageFilter _usBlurFilter;
        AdvancedUsFusion _usFusion;

        BoolProperty p_autoExecution;

        StringProperty p_sourcePath;
        StringProperty p_targetPathColor;
        StringProperty p_targetPathFuzzy;
        IVec2Property p_range;

        ButtonProperty p_execute;
    };

}

#endif // CMBATCHGENERATION_H__
