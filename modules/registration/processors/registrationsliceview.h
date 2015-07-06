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

#ifndef REGISTRATIONSLICEVIEW_H__
#define REGISTRATIONSLICEVIEW_H__

#include <string>

#include "core/pipeline/slicerenderprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace campvis {
namespace registration {

    /**
     * Renders the slice of the moving image with respect to the fixed image for registration purposes.
     */
    class CAMPVIS_MODULES_API RegistrationSliceView : public SliceRenderProcessor {
    public:
        /**
         * Constructs a new RegistrationSliceView Processor
         **/
        RegistrationSliceView(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~RegistrationSliceView();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "RegistrationSliceView"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders the slice of the moving image with respect to the fixed image for registration purposes."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_movingImage;                       ///< ID for moving image
        DataNameProperty p_movingTransformationMatrix;          ///< ID for TransformData of moving image

        TransferFunctionProperty p_referenceTransferFunction;   ///< Transfer function for first image
        TransferFunctionProperty p_movingTransferFunction;      ///< Transfer function for second image

    protected:
        /// \see SliceRenderProcessor::renderImageImpl
        virtual void renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img) override;
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer) override;

    private:
        static const std::string loggerCat_;
    };

}
}

#endif // REGISTRATIONSLICEVIEW_H__
