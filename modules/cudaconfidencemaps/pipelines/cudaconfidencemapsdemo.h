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

#ifndef CUDACONFIDENCEMAPSDEMO_H__
#define CUDACONFIDENCEMAPSDEMO_H__

#include <tbb/tick_count.h>


#include "core/pipeline/autoevaluationpipeline.h"
#include "core/properties/buttonproperty.h"

#include "modules/modulesapi.h"
#include "modules/base/processors/matrixprocessor.h"
#include "modules/preprocessing/processors/glgaussianfilter.h"
#include "modules/preprocessing/processors/glimagecrop.h"
#include "modules/preprocessing/processors/glimageresampler.h"
#include "modules/cudaconfidencemaps/processors/cudaconfidencemapssolver.h"
#include "modules/cudaconfidencemaps/processors/usfanrenderer.h"
#include "modules/openigtlink/processors/openigtlinkclient.h"
#include "modules/advancedusvis/processors/advancedusfusion.h"



namespace campvis {

    class CAMPVIS_MODULES_API CudaConfidenceMapsDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        CudaConfidenceMapsDemo(DataContainer *dc);

        /**
         * Virtual Destructor
         **/
        virtual ~CudaConfidenceMapsDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::executePipeline()
        virtual void executePipeline();

        /// \see AbstractPipeline::onEvent()
        virtual void onEvent(cgt::Event* e);

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "CudaConfidenceMapsDemo"; };

    protected:
        virtual void onPropertyChanged(const AbstractProperty* prop);

        void toggleIGTLConnection();
        void copyStatisticsToClipboard();
        void setAdvancedPropertiesVisibility(bool visible);

        // Processors
        OpenIGTLinkClient _usIgtlReader;
        GlImageCrop       _usCropFilter;
        GlGaussianFilter  _usBlurFilter;
        GlImageResampler  _usResampler;
        CudaConfidenceMapsSolver _usMapsSolver;
        AdvancedUsFusion  _usFusion;
        UsFanRenderer     _usFanRenderer;

        // Basic options
        BoolProperty   p_useFixedIterationCount;
        FloatProperty  p_millisecondBudget;
        IntProperty    p_iterationBudget;
        ButtonProperty p_connectDisconnectButton;
        FloatProperty  p_resamplingScale;
        FloatProperty  p_beta;

        BoolProperty   p_collectStatistics;
        ButtonProperty p_copyStatisticsToClipboard;

        BoolProperty   p_showAdvancedOptions;

        // Advanced options (hidden by default)
        BoolProperty   p_useAlphaBetaFilter;
        FloatProperty  p_gaussianFilterSize;
        FloatProperty  p_gradientScaling;
        FloatProperty  p_alpha;
        FloatProperty  p_gamma;
        FloatProperty  p_fanHalfAngle;
        FloatProperty  p_fanInnerRadius;
        BoolProperty   p_useSpacingEncodedFanGeometry; // Fan geometry is transmitted to campvis encoded in the x and y component of the voxel size (FOV in deg, innerRadius)
        StringProperty p_recordingDirectory;
        BoolProperty   p_enableRecording;

        // Data structures to collect statistics
        struct StatisticsEntry {
            float time;
            int originalWidth, originalHeight;
            int downsampledWidth, downsampledHeight;
            float gaussianKernelSize;
            float scalingFactor;
            float alpha, beta, gamma, gradientScaling;
            int iterations;
            float solverExecutionTime;
            float totalExecutionTime;
            float solverError;
        };
        std::vector<StatisticsEntry> _statistics;
        tbb::tick_count _objectCreationTime;

        // Variables to keep track of file naming when recording...
        int _recordedFrames;
        std::string _filePrefix;

        tbb::tick_count _statisticsLastUpdateTime;
    };

}

#endif // CUDACONFIDENCEMAPSDEMO_H__
