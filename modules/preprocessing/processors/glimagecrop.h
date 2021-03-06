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

#ifndef GLIMAGECROP_H__
#define GLIMAGECROP_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/properties/numericproperty.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Quantizes image intensities into a fixed number of bins using the GPU.
     */
    class GlImageCrop : public VisualizationProcessor {
    public:
        /**
         * Constructs a new GlImageCrop Processor
         **/
        explicit GlImageCrop(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GlImageCrop();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "GlImageCrop"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Quantizes image intensities into a fixed number of bins using the GPU."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;      ///< ID for input volume
        DataNameProperty p_outputImage;     ///< ID for output gradient volume

        IVec3Property p_llf;                ///< LLF of cropped region
        IVec3Property p_urb;                ///< URB of cropped region

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        cgt::Shader* _shader2D;             ///< Shader for cropping 2D data
        cgt::Shader* _shader3D;             ///< Shader for cropping 3D data

        static const std::string loggerCat_;
    };
}

#endif // GLIMAGECROP_H__
