// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef STRAINRAYCASTER_H__
#define STRAINRAYCASTER_H__

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include <string>

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Performs strain volume ray casting.
     */
    class StrainRaycaster : public RaycastingProcessor {
    public:
        /**
         * Constructs a new StrainRaycaster Processor
         **/
        StrainRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~StrainRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "StrainRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a simple volume ray casting."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_targetImageID;    ///< image ID for output image
        BoolProperty p_enableShadowing;
        FloatProperty p_shadowIntensity;
        BoolProperty p_enableAdaptiveStepsize;

    
    protected:
        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);

        /// \see RaycastingProcessor::generateHeader()
        virtual std::string generateHeader() const;

        static const std::string loggerCat_;
    };

}

#endif // STRAINRAYCASTER_H__
