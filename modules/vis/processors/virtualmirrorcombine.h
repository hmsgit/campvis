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

#ifndef VIRTUALMIRRORCOMBINE_H__
#define VIRTUALMIRRORCOMBINE_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Performs the composition of a rendering, a virtual mirror rendering and the corresponding mirror.
     */
    class CAMPVIS_MODULES_API VirtualMirrorCombine : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new VirtualMirrorCombine Processor
         **/
        explicit VirtualMirrorCombine(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~VirtualMirrorCombine();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "VirtualMirrorCombine"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines Normal DVR and Virtual Mirror DVR images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_normalImageID;            ///< image ID for normal DVR input image
        DataNameProperty p_mirrorImageID;            ///< image ID for mirror DVR input image
        DataNameProperty p_mirrorRenderID;           ///< image ID for rendered mirror input image
        DataNameProperty p_targetImageID;            ///< image ID for output image

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        cgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };
}

#endif // VIRTUALMIRRORCOMBINE_H__
