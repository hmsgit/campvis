// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#ifndef ITKSEGMENTATION_H__
#define ITKSEGMENTATION_H__

#include <string>

#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/vis/processors/volumeexplorer.h"

namespace campvis {
    /**
    * Performs segmentation on input image using ITK.
    */
    class ItkSegmentation : public VolumeExplorer {
    public:
        /**
        * Constructs a new ItkSegmentation Processor
        **/
        ItkSegmentation(IVec2Property* viewportSizeProp);

        /**
        * Destructor
        **/
        virtual ~ItkSegmentation();

        /// \see AbstractEventHandler::execute()
        virtual void onEvent(cgt::Event* e);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ItkSegmentation"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a segmentation on input image using ITK."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Cristina Precup <cristina.precup@tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageID;   ///< ID for input volume
        DataNameProperty p_targetImageID;   ///< ID for segmented output volume

        GenericOptionProperty<std::string> p_segmentationType;    ///< Segmentation type

        IntProperty p_seedX;
        IntProperty p_seedY;
        IntProperty p_seedZ;
        IntProperty p_thresMin;
        IntProperty p_thresMax;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // ITKSEGMENTATION_H__
