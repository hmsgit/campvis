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

#ifndef ADVANCEDUSVIS_H__
#define ADVANCEDUSVIS_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
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

namespace campvis {
    class AdvancedUsVis : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline. 
         */
        AdvancedUsVis(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~AdvancedUsVis();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see VisualizationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "AdvancedUsVis"; };

        /**
         * Execute this pipeline.
         **/
        virtual void execute();

        virtual void keyEvent(tgt::KeyEvent* e);

    protected:

        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        CameraProperty _camera;

        MhdImageReader _usReader;
        MhdImageReader _confidenceReader;
        ConfidenceMapGenerator _confidenceGenerator;

        GradientVolumeGenerator _gvg;
        LHHistogram _lhh;
        AdvancedUsFusion _usFusion1;
        AdvancedUsFusion _usFusion2;
        AdvancedUsFusion _usFusion3;
        AdvancedUsFusion _usFusion4;
        ItkImageFilter _usBlurFilter;

        QuadView _quadView;

        ItkImageFilter _usDenoiseilter;
        ProxyGeometryGenerator _usProxy;
        EEPGenerator _usEEP;
        SimpleRaycaster _usDVR;

        MWheelToNumericPropertyEventListener _wheelHandler;
        TransFuncWindowingEventListener _tfWindowingHandler;

        TrackballNavigationEventListener* _trackballEH;
    };

}

#endif // ADVANCEDUSVIS_H__
