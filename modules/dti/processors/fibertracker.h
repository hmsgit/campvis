// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef FIBERTRACKER_H__
#define FIBERTRACKER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"

#include "modules/modulesapi.h"

#include <deque>

namespace campvis {
    class ImageRepresentationLocal;

namespace dti {

    /**
     * Performs a fiber tracking of incoming strain data.
     */
    class CAMPVIS_MODULES_API FiberTracker : public AbstractProcessor {
    public:
        /**
         * Constructs a new FiberTracker Processor
         **/
        FiberTracker();

        /**
         * Destructor
         **/
        virtual ~FiberTracker();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "FiberTracker"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a fiber tracking of incoming strain data."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_strainId;   ///< image ID for input strain data
        DataNameProperty p_outputID;   ///< image ID for output fiber data

        ButtonProperty p_updateButton;      ///< Button to start tracking

        IntProperty p_seedDistance;

        IntProperty p_numSteps;             ///< Maximum number of steps per fiber
        FloatProperty p_stepSize;           ///< Base step size

        FloatProperty p_strainThreshold;    ///< Local strain threshold (minimum) to perform tracking
        FloatProperty p_maximumAngle;       ///< maximum angle between two adjacent fiber segments

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        /**
         * Creates seed points uniformly spread over volume and writes them to \a seeds.
         * \param   strainData  Input strain data
         * \return  vector of seed points in voxel coordinates
         **/
        std::vector<cgt::vec3> performUniformSeeding(const ImageRepresentationLocal& strainData) const;

        /**
         * Retrieves a vec3 from \a vol using trilinear interpolation.
         *
         * \param   vol             volume to get data from
         * \param   position        voxel position
         **/
        inline cgt::vec3 getVec3FloatLinear(const ImageRepresentationLocal& strainData, const cgt::vec3& position) const;

        static const std::string loggerCat_;
    };

}
}

#endif // FIBERTRACKER_H__
