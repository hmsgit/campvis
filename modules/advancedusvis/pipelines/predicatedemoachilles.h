// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef PredicateDemoAchilles_H__
#define PredicateDemoAchilles_H__

#include "core/pipeline/autoevaluationpipeline.h"
#include "core/properties/cameraproperty.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/advancedusvis/processors/pointpredicateevaluator.h"
#include "modules/advancedusvis/processors/predicatevolumeexplorer.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/preprocessing/processors/glimageresampler.h"
#include "modules/preprocessing/processors/glintensityquantizer.h"
#include "modules/preprocessing/processors/glgaussianfilter.h"
#include "modules/preprocessing/processors/glgradientvolumegenerator.h"
#include "modules/preprocessing/processors/glmorphologyfilter.h"
#include "modules/preprocessing/processors/glsignaltonoiseratiofilter.h"
#include "modules/preprocessing/processors/glvesselnessfilter.h"

#include "core/datastructures/genericimagerepresentationlocal.h"


namespace campvis {
    class PredicateDemoAchilles : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        PredicateDemoAchilles(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~PredicateDemoAchilles();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "PredicateDemoAchilles"; };

        void onRenderTargetSizeChanged(const AbstractProperty* prop);

    protected:
        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        LightSourceProvider _lsp;
        MhdImageReader _imageReader;
        MhdImageReader _labelReader;
        MhdImageReader _confidenceReader;
        
        GlGaussianFilter _gaussian;
        GlVesselnessFilter _vesselnesFilter;

        GlSignalToNoiseRatioFilter _snrFilter;

        PredicateVolumeExplorer _ve;

    };

}

#endif // PredicateDemoAchilles_H__
