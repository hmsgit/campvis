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

#ifndef IXPVCOMPOSITOR_H__
#define IXPVCOMPOSITOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Performs IXPV composition of a 3D slice rendering and a X-Ray image.
     */
    class IxpvCompositor : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new IxpvCompositor Processor
         **/
        IxpvCompositor(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~IxpvCompositor();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "IxpvCompositor"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs IXPV composition of a 3D volume rendering and a X-Ray image."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_xRayImageId;             ///< image ID for X-Ray input image
        DataNameProperty p_3dSliceImageId;          ///< image ID for the slice input image rendered in 3D world space
        DataNameProperty p_drrFullImageId;          ///< image ID full DRR input image
        DataNameProperty p_drrClippedImageId;       ///< image ID clipped DRR input image
        DataNameProperty p_targetImageId;           ///< image ID for output image

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // IXPVCOMPOSITOR_H__
