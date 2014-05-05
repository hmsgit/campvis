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

#ifndef POINTPREDICATERAYCASTER_H__
#define POINTPREDICATERAYCASTER_H__

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"

#include "modules/advancedusvis/properties/pointpredicatehistogramproperty.h"

#include <string>

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Performs a predicate-based volume ray casting.
     */
    class PointPredicateRaycaster : public RaycastingProcessor {
    public:
        /**
         * Constructs a new PointPredicateRaycaster Processor
         **/
        PointPredicateRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~PointPredicateRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "PointPredicateRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a predicate-based volume ray casting"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        DataNameProperty p_inputLabels;
        DataNameProperty p_inputSnr;
        DataNameProperty p_inputVesselness;
        DataNameProperty p_inputConfidence;

        BoolProperty p_enableShading;               ///< Flag whether to enable shading
        DataNameProperty p_lightId;                 ///< Name/ID for the LightSource to use
        PointPredicateHistogramProperty p_predicateHistogram;
            
    protected:
        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);
        /// \see AbstractProcessor::updateProperties()
        virtual void updateProperties(DataContainer& dataContainer);

        /// \see RaycastingProcessor::generateHeader()
        virtual std::string generateHeader() const;

    private:
        void onHistogramHeaderChanged();

        tgt::Shader* _clusterShader;

        static const std::string loggerCat_;
    };

}

#endif // POINTPREDICATERAYCASTER_H__
