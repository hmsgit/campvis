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

#ifndef DEVILIMAGEREADER_H__
#define DEVILIMAGEREADER_H__

#include <string>

#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/stringproperty.h"

#include "modules/modulesapi.h"
#include "modules/io/processors/abstractimagereader.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Reads an image file into the pipeline using the DevIL library.
     * DevIL supports most common 2D image formats. 
     *
     * \note    Full list of supported formats: http://openil.sourceforge.net/features.php
     */
    class CAMPVIS_MODULES_API DevilImageReader : public AbstractImageReader {
    public:
        /**
         * Constructs a new DevilImageReader Processor
         **/
        DevilImageReader();

        /**
         * Destructor
         **/
        virtual ~DevilImageReader();


        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DevilImageReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads an image file into the pipeline using the DevIL library."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        GenericOptionProperty<std::string> p_importType;
        BoolProperty p_importSimilar;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        cgt::Shader* _shader;

        static const std::string loggerCat_;
    };

}

#endif // DEVILIMAGEREADER_H__
