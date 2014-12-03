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

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class CAMPVIS_MODULES_API AdvancedUsFusion : public VisualizationProcessor, public HasProcessorDecorators {
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
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

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
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// adapts the range of the p_sliceNumber property to the image
        virtual void updateProperties(DataContainer& dc);

        std::string generateHeader() const;

        cgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // ADVANCEDUSFUSION_H__
