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

#ifndef RENDERTARGETCOMPOSITOR_H__
#define RENDERTARGETCOMPOSITOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Performs the composition of a multiple render targets
     */
    class RenderTargetCompositor : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        enum CompositingMode {
            CompositingModeFirst = 0,
            CompositingModeSecond = 1,
            CompositingModeAlpha = 2,
            CompositingModeDifference = 3,
            CompositingModeDepth = 4
        };

        /**
         * Constructs a new RenderTargetCompositor Processor
         **/
        RenderTargetCompositor(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~RenderTargetCompositor();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "RenderTargetCompositor"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines Normal DVR and Virtual Mirror DVR images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual const ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        virtual void process(DataContainer& data);

        DataNameProperty p_firstImageId;             ///< image ID for the first image to combine
        DataNameProperty p_secondImageId;            ///< image ID for the second image to combine
        DataNameProperty p_targetImageId;            ///< image ID for combined output image
        GenericOptionProperty<CompositingMode> p_compositingMethod;
        FloatProperty p_alphaValue;

    protected:
        tgt::Shader* _shader;                           ///< Shader for composition

        static const std::string loggerCat_;
    };

}

#endif // RENDERTARGETCOMPOSITOR_H__
