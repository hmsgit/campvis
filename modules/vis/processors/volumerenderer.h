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

#ifndef VOLUMERENDERER_H__
#define VOLUMERENDERER_H__

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/metaproperty.h"
#include "core/properties/numericproperty.h"

#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/simpleraycaster.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Combines proxy geometry generator, entry/exit point generator and raycasting into one
     * easy-to-use volume renderer.
     */
    class VolumeRenderer : public VisualizationProcessor {
    public:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            PG_INVALID = 1 << 4,
            EEP_INVALID = 1 << 5,
            RAYCASTER_INVALID = 1 << 6
        };

        /**
         * Constructs a new VolumeRenderer Processor
         **/
        VolumeRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~VolumeRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VolumeRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines proxy geometry generator, entry/exit point generator and raycasting into one easy-to-use volume renderer."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /// \see VisualizationPipeline::setViewportSizeProperty()
        virtual void setViewportSizeProperty(IVec2Property* viewportSizeProp);

        virtual void process(DataContainer& data);

        DataNameProperty p_inputVolume;              ///< image ID for first input image
        CameraProperty p_camera;
        DataNameProperty p_outputImage;              ///< image ID for output image

        MetaProperty p_pgProps;                     ///< MetaProperty for properties of the ProxyGeometryGenerator processor
        MetaProperty p_eepProps;                    ///< MetaProperty for properties of the EEPGenerator processor
        MetaProperty p_raycasterProps;              ///< MetaProperty for properties of the raycasting processor

    protected:
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

        ProxyGeometryGenerator _pgGenerator;
        EEPGenerator _eepGenerator;
        SimpleRaycaster _raycaster;

        static const std::string loggerCat_;
    };

}

#endif // VOLUMERENDERER_H__
