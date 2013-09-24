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

#ifndef IXPVDEMO_H__
#define IXPVDEMO_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/properties/cameraproperty.h"
#include "core/pipeline/visualizationpipeline.h"

#include "modules/io/processors/mhdimagereader.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/vis/processors/sliceextractor.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/geometryrenderer.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/drrraycaster.h"
#include "modules/vis/processors/simpleraycaster.h"
#include "modules/vis/processors/rendertargetcompositor.h"
#include "modules/vis/processors/slicerenderer3d.h"
#include "modules/vis/processors/ixpvcompositor.h"


namespace campvis {
    class IxpvDemo : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        IxpvDemo();

        /**
         * Virtual Destructor
         **/
        virtual ~IxpvDemo();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const;


        //virtual void keyEvent(tgt::KeyEvent* e);

        void onRenderTargetSizeChanged(const AbstractProperty* prop);
    protected:
        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        DevilImageReader _xrayReader;

        MhdImageReader _ctReader;
        ProxyGeometryGenerator _ctProxy;
        EEPGenerator _ctFullEEP;
        EEPGenerator _ctClippedEEP;
        SimpleRaycaster _ctDVR;
        DRRRaycaster _ctFullDRR;
        DRRRaycaster _ctClippedDRR;

        MhdImageReader _usReader;
        SliceRenderer3D _usSliceRenderer;

        RenderTargetCompositor _compositor;
        RenderTargetCompositor _compositor2;
        IxpvCompositor _ixpvCompositor;

        CameraProperty _camera;

        TrackballNavigationEventListener* _trackballHandler;
        MWheelToNumericPropertyEventListener _wheelHandler;
    };
}

#endif // IXPVDEMO_H__

