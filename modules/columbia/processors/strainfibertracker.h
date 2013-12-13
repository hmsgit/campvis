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

#ifndef STRAINFIBERTRACKER_H__
#define STRAINFIBERTRACKER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"

#include <deque>

namespace campvis {
    class ImageRepresentationLocal;

    /**
     * Performs a fiber tracking of incomoing strain data.
     */
    class StrainFiberTracker : public AbstractProcessor {
    public:
        /**
         * Constructs a new StrainFiberTracker Processor
         **/
        StrainFiberTracker();

        /**
         * Destructor
         **/
        virtual ~StrainFiberTracker();


        /**
         * Reads the raw file into an ImageRepresentationDisk representation
         * \param data  DataContainer to work on
         */
        virtual void process(DataContainer& data);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "StrainFiberTracker"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a fiber tracking of incomoing strain data."; };
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
        /**
         * Creates seed points uniformly spread over volume and writes them to \a seeds.
         * \param   strainData  Input strain data
         * \return  vector of seed points in voxel coordinates
         **/
        std::vector<tgt::vec3> performUniformSeeding(const ImageRepresentationLocal& strainData) const;

        /**
         * Retrieves a vec3 from \a vol using trilinear interpolation.
         *
         * \param   vol             volume to get data from
         * \param   position        voxel position
         **/
        inline tgt::vec3 getVec3FloatLinear(const ImageRepresentationLocal& strainData, const tgt::vec3& position) const;

        static const std::string loggerCat_;
    };

}

#endif // STRAINFIBERTRACKER_H__
