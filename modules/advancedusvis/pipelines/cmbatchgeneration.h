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

#ifndef CMBATCHGENERATION_H__
#define CMBATCHGENERATION_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/devil/processors/devilimagewriter.h"
#include "modules/advancedusvis/processors/advancedusfusion.h"
#include "modules/advancedusvis/processors/scanlineconverter.h"
#include "modules/preprocessing/processors/glgaussianfilter.h"
#include "modules/randomwalk/processors/confidencemapgenerator.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    
    class CAMPVIS_MODULES_API CmBatchGeneration : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a CmBatchGeneration pipeline.
         * \param   dataContainer   Reference to the DataContainer containing local working set of data
         *                          for this pipeline, must be valid the whole lifetime of this pipeline.
         */
        explicit CmBatchGeneration(DataContainer& dataContainer);

        /**
         * Virtual Destructor
         **/
        virtual ~CmBatchGeneration();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();
        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        static const std::string getId() { return "CmBatchGeneration"; };

        virtual void paint() override;

    protected:
        void onPropertyChanged(const AbstractProperty* p) override;
        virtual void onProcessorInvalidated(AbstractProcessor* processor) override;

        void startBatchProcess();
        void executePass(int path);
        void save(const std::string& dataName, const std::string& fileName);

        DevilImageReader _usReader;                     ///< Reads the original image
        ScanlineConverter _scanlineConverter;           ///< Performs a scanline conversion
        ConfidenceMapGenerator _confidenceGenerator;    ///< Computes the CM using the original RandomWalks library
        GlGaussianFilter _usBlurFilter;                 ///< Performs a Gaussian Blur
        AdvancedUsFusion _usFusion;                     ///< Applies the Uncertainty Visualization

        DevilImageWriter _imageWriter;                  ///< Used to write out images

        BoolProperty p_autoExecution;
        BoolProperty p_showFan;

        StringProperty p_sourcePath;                    ///< Path for the input images
        StringProperty p_targetPathResampled;           ///< Path for the resampled images
        StringProperty p_targetPathCmCpu;               ///< Path for the CPU-computed Confidence Maps
        StringProperty p_targetPathColorOverlay;        ///< Path for the color overlay visualization
        StringProperty p_targetPathColor;               ///< Path for the color modulation visualization
        StringProperty p_targetPathFuzzy;               ///< Path for the fuzziness visualization

        IVec2Property p_range;                          ///< Range for image iteration
        ButtonProperty p_execute;                       ///< Button to start the batch process

        cgt::Shader* _shader;
    };
}

#endif // CMBATCHGENERATION_H__
