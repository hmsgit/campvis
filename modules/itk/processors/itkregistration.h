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

#ifndef ITKREGISTRATION_H__
#define ITKREGISTRATION_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/preprocessing/tools/abstractimagefilter.h"

namespace campvis {
    /**
    * Performs watershed image filter on input image using ITK.
    */
    class ItkRegistration : public AbstractProcessor {
    public:
        /**
        * Constructs a new ItkRegistration Processor
        **/
        ItkRegistration();

        /**
        * Destructor
        **/
        virtual ~ItkRegistration();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ItkRegistration"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs registration between 2 input images using ITK."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Cristina Precup <cristina.precup@tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageIDFixed;   ///< ID for fixed input volume
        DataNameProperty p_sourceImageIDMoving;   ///< ID for moving input volume
        DataNameProperty p_targetImageID;   ///< ID for output volume

        GenericOptionProperty<std::string> p_registrationType;    ///< Registration type

        IntProperty p_noOfBins;
        IntProperty p_noOfSamples;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // ITKREGISTRATION_H__
