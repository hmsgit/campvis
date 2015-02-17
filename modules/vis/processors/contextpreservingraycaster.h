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

#ifndef CONTEXTPRESERVINGRAYCASTER_H__
#define CONTEXTPRESERVINGRAYCASTER_H__

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "modules/modulesapi.h"
#include "modules/processorfactory.h"

#include <string>

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Performs a illustrative context-preserving volume ray casting.
     * Implementation of "Illustrative Context-Preserving Volume Rendering" by Bruckner et al., IEEE VGTC 2005
     */
    class CAMPVIS_MODULES_API ContextPreservingRaycaster : public RaycastingProcessor {
    public:
        /**
         * Constructs a new ContextPreservingRaycaster Processor
         **/
        ContextPreservingRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~ContextPreservingRaycaster();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "ContextPreservingRaycaster"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a simple volume ray casting."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        BoolProperty p_enableShading;   ///< Flag whether to enable shading
        DataNameProperty p_lightId;     ///< Name/ID for the LightSource to use

        FloatProperty p_kappaS;     ///< k_s parameter from the paper
        FloatProperty p_kappaT;     ///< l_t parameter from the paper
    
    protected:
        /// \see AbstractProcessor::updateProperties()
        virtual void updateProperties(DataContainer& dataContainer);

        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);

        /// \see RaycastingProcessor::generateHeader()
        virtual std::string generateHeader() const;

        static const std::string loggerCat_;
    };

    // Instantiate template to register the pipelines.
    template class SmartProcessorRegistrar<ContextPreservingRaycaster>;
}

#endif // CONTEXTPRESERVINGRAYCASTER_H__
