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

#ifndef SLICEEXTRACTOR_H__
#define SLICEEXTRACTOR_H__

#include "core/pipeline/slicerenderprocessor.h"
#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class CAMPVIS_MODULES_API SliceExtractor : public SliceRenderProcessor {
    public:
        /**
         * Constructs a new SliceExtractor Processor
         **/
        explicit SliceExtractor(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~SliceExtractor();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "SliceExtractor"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        TransferFunctionProperty p_transferFunction;        ///< Transfer function

    protected:
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see SliceRenderProcessor::renderImageImpl
        virtual void renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img);

        static const std::string loggerCat_;
    };
}

#endif // SLICEEXTRACTOR_H__
