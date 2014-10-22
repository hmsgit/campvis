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

#ifndef LIGHTSOURCEPROVIDER_H__
#define LIGHTSOURCEPROVIDER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Creates LightSourceData and stores it in the DataContainer.
     * \see LightSourceData
     */
    class CAMPVIS_MODULES_API LightSourceProvider : public AbstractProcessor {
    public:
        /**
         * Constructs a new LightSourceProvider Processor.
         **/
        LightSourceProvider();

        /**
         * Destructor
         **/
        virtual ~LightSourceProvider();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "LightSourceProvider"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates LightSourceData and stores it in the DataContainer."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::STABLE; };

        DataNameProperty p_lightId;         ///< Name/ID for the LightSource to create
        Vec3Property p_lightPosition;       ///< Light position
        Vec3Property p_ambientColor;        ///< Ambient light color
        Vec3Property p_diffuseColor;        ///< Diffuse light color
        Vec3Property p_specularColor;       ///< Specular light color
        FloatProperty p_shininess;          ///< Specular shininess
        Vec3Property p_attenuation;         ///< Attenuation factors

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // LIGHTSOURCEPROVIDER_H__
