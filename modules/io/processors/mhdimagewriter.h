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

#ifndef MHDIMAGEWRITER_H__
#define MHDIMAGEWRITER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/stringproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Writes an image into an MHD file.
     *
     * \note    Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
     */
    class CAMPVIS_MODULES_API MhdImageWriter : public AbstractProcessor {
    public:
        /**
         * Constructs a new MhdImageWriter Processor
         **/
        MhdImageWriter();

        /**
         * Destructor
         **/
        virtual ~MhdImageWriter();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "MhdImageWriter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Writes an image into an MHD file."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_inputImage;
        StringProperty p_fileName;
        ButtonProperty p_saveFile;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // MHDIMAGEWRITER_H__
