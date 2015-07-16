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

#ifndef ITKWATERSHEDFILTER_H__
#define ITKWATERSHEDFILTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"
#include "modules/preprocessing/tools/abstractimagefilter.h"

namespace campvis {
    /**
     * Performs watershed image filter on input image using ITK.
     */
    class CAMPVIS_MODULES_API ItkWatershedFilter : public AbstractProcessor {
    public:
        /**
         * Constructs a new ItkWatershedFilter Processor
         **/
        ItkWatershedFilter();

        /**
         * Destructor
         **/
        virtual ~ItkWatershedFilter();

        /// To be used in ProcessorFactory static methods
        static const std::string getId() { return "ItkWatershedFilter"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs watershed image filter on input image using ITK."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_sourceImageID;   ///< ID for input volume
        DataNameProperty p_targetImageID;   ///< ID for output gradient volume

        FloatProperty p_threshold;
        FloatProperty p_level;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // ITKWATERSHEDFILTER_H__
