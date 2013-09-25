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

#ifndef ADVANCEDUSFUSION_H__
#define ADVANCEDUSFUSION_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class AdvancedUsFusion : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new AdvancedUsFusion Processor
         **/
        AdvancedUsFusion(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~AdvancedUsFusion();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "AdvancedUsFusion"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual const ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::process()
        virtual void process(DataContainer& data);

        DataNameProperty p_usImageId;                  ///< image ID for input image
        DataNameProperty p_blurredImageId;
        DataNameProperty p_gradientImageID;
        DataNameProperty p_confidenceImageID;
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        IntProperty p_sliceNumber;                       ///< number of the slice to extract
        TransferFunctionProperty p_transferFunction;     ///< Transfer function
        TransferFunctionProperty p_confidenceTF;         ///< Transfer function confidence to uncertainty
        GenericOptionProperty<std::string> p_view;
        FloatProperty p_blurredScaling;
        FloatProperty p_confidenceScaling;
        FloatProperty p_hue;
        BoolProperty p_use3DTexture;

    protected:
        /// adapts the range of the p_sliceNumber property to the image
        void updateProperties(DataHandle img);

        std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;

    private:
        clock_t _sourceImageTimestamp;
    };

}

#endif // ADVANCEDUSFUSION_H__
