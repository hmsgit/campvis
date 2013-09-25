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

#ifndef COLUMBIA1_H__
#define COLUMBIA1_H__

#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/properties/cameraproperty.h"


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
    class Columbia1 : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        Columbia1();

        /**
         * Virtual Destructor
         **/
        virtual ~Columbia1();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see VisualizationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const;


    protected:
        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        CameraProperty _camera;
        DataNameProperty _boundsData;

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

        TrackballNavigationEventListener* _trackballEH;

    };
}

#endif // COLUMBIA1_H__
