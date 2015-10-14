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

#ifndef FIBERREADER_H__
#define FIBERREADER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace campvis {
namespace dti {
    class FiberData;

    /**
     * Reads Fiber Data into the DataContainer.
     */
    class CAMPVIS_MODULES_API FiberReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new FiberReader Processor
         **/
        FiberReader();

        /**
         * Destructor
         **/
        virtual ~FiberReader();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "FiberReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads Fiber Data into the DataContainer."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        StringProperty p_url;                       ///< Input file name URL
        DataNameProperty p_outputId;                ///< ID for output fiber data

        Vec3Property p_offset;                      ///< Additional Offset
        Vec3Property p_scaling;                     ///< Additional scaling

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        FiberData* readTrkFile(const std::string& fileName);

        static const std::string loggerCat_;
    };

}
}

#endif // FIBERREADER_H__
