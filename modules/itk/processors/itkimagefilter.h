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

#ifndef ITKIMAGEFILTER_H__
#define ITKIMAGEFILTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/preprocessing/tools/abstractimagefilter.h"

namespace campvis {
    /**
     * Performs different filter operations on images.
     */
    class ItkImageFilter : public AbstractProcessor {
    public:
        /**
         * Constructs a new ItkImageFilter Processor
         **/
        ItkImageFilter();

        /**
         * Destructor
         **/
        virtual ~ItkImageFilter();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ItkImageFilter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates the gradient volume for the given intensity volume."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_sourceImageID;   ///< ID for input volume
        DataNameProperty p_targetImageID;   ///< ID for output gradient volume

        GenericOptionProperty<std::string> p_filterMode;    ///< Filter mode
        IntProperty p_kernelSize;
        FloatProperty p_sigma;
        IntProperty p_numberOfSteps;
        FloatProperty p_timeStep;
        FloatProperty p_conductance;
        FloatProperty p_thresMin;
        FloatProperty p_thresMax;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // ITKIMAGEFILTER_H__
