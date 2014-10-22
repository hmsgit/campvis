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

#ifndef IVUSBATCHREADER_H__
#define IVUSBATCHREADER_H__

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/devil/processors/devilimagereader.h"

namespace campvis {

    /**
     * Reads a batch of IVUS images from a directory and stacks them into a 3D volume.
     */
    class CAMPVIS_MODULES_API IvusBatchReader : public VisualizationProcessor {
    public:
        /**
         * Constructs a new IvusBatchReader Processor
         **/
        IvusBatchReader(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~IvusBatchReader();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "IvusBatchReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads a batch of IVUS images from a directory and stacks them into a 3D volume."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        StringProperty p_inputDirectory;            ///< Input directory to read files from
        StringProperty p_fileExtension;             ///< File extension of images to read

        Vec3Property p_imageSpacing;                     ///< Image spacing
        DataNameProperty p_outputImage;             ///< image ID for output image


    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // IVUSBATCHREADER_H__
