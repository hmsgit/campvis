// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef CMBATCHGENERATION_H__
#define CMBATCHGENERATION_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/pipeline/visualizationpipeline.h"
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
    class CmBatchGeneration : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline. 
         */
        CmBatchGeneration();

        /**
         * Virtual Destructor
         **/
        virtual ~CmBatchGeneration();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see VisualizationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const;

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
