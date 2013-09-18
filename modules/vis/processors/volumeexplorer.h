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

#ifndef VOLUMEEXPLORER_H__
#define VOLUMEEXPLORER_H__

#include "core/eventhandlers/abstracteventhandler.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventhandler.h"
#include "core/eventhandlers/trackballnavigationeventhandler.h"
#include "core/eventhandlers/transfuncwindowingeventhandler.h"

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

#include "modules/vis/processors/volumerenderer.h"
#include "modules/vis/processors/sliceextractor.h"

namespace tgt {
    class Shader;
}

class FaceGeometry;

namespace campvis {
    /**
     * Combines a volume raycaster and 3 slice views for explorative volume visualization.
     */
    class VolumeExplorer : public VisualizationProcessor, public AbstractEventHandler {
    public:
        /**
         * Constructs a new VolumeExplorer Processor
         **/
        VolumeExplorer(IVec2Property& canvasSize);

        /**
         * Destructor
         **/
        virtual ~VolumeExplorer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VolumeExplorer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines a volume raycaster and 3 slice views for explorative volume visualization."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual const ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractEventHandler::accept()
        virtual bool accept(tgt::Event* e);
        /// \see AbstractEventHandler::execute()
        virtual void execute(tgt::Event* e);

        virtual void process(DataContainer& data);

        DataNameProperty p_inputVolume;              ///< image ID for first input image

        CameraProperty p_camera;
        IntProperty p_xSlice;
        IntProperty p_ySlice;
        IntProperty p_zSlice;

        DataNameProperty p_outputImage;              ///< image ID for output image

    protected:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            VR_INVALID = 1 << 4,
            SLICES_INVALID = 1 << 5,
        };

        /**
         * Slot getting called when one of the observed processors got invalidated.
         * Invalidates this meta-processor with the corresponding level.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);

        /**
         * \see VisualizationProcessor::onPropertyChanged
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        void composeFinalRendering(DataContainer& data);

        /// adapts the range of the p_xSliceNumber property to the image
        void updateProperties(DataHandle img);

        tgt::Shader* _shader;                           ///< Shader for slice rendering
        FaceGeometry* _quad;

        VolumeRenderer _raycaster;
        SliceExtractor _sliceExtractor;

        IVec2Property p_sliceRenderSize;
        IVec2Property p_volumeRenderSize;


        MWheelToNumericPropertyEventHandler _xSliceHandler;
        MWheelToNumericPropertyEventHandler _ySliceHandler;
        MWheelToNumericPropertyEventHandler _zSliceHandler;
        TransFuncWindowingEventHandler _windowingHandler;
        TrackballNavigationEventHandler* _trackballEH;

        static const std::string loggerCat_;
    };

}

#endif // VOLUMEEXPLORER_H__
