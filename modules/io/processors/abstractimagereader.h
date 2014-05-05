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

#ifndef ABSTRACTIMAGEREADER_H__
#define ABSTRACTIMAGEREADER_H__

#include <string>
#include <vector>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/stringproperty.h"

namespace campvis {
    /**
     * Reads an image file into the pipeline. This is mainly a wrapper class. It uses
     * the other image reader implemented for its tasks.
     *
     * Provides the interface for the classes that reads images from different types
     * of files into ImageRepresentationDisk representation
     *
     */
    class AbstractImageReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new AbstractImageReader Processor
         **/
        AbstractImageReader();

        /**
         * Destructor
         **/
        virtual ~AbstractImageReader();

        /**
         * Checks whether an extension is handled by the current reader.
         * returns true is the extension is handled, false otherwise
         *
         * \param extension  The extension to be checked
         **/
        virtual bool acceptsExtension(const std::string& extension) const;
        
        StringProperty p_url;               ///< URL for file to read
        DataNameProperty p_targetImageID;   ///< image ID for read image

    protected:
        std::vector<std::string> _ext;      ///< accepted extensions

    };

}

#endif // ABSTRACTIMAGEREADER_H__
