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

#ifndef EEPGENERATOR_H__
#define EEPGENERATOR_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class CAMPVIS_MODULES_API EEPGenerator : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new EEPGenerator Processor
         **/
        EEPGenerator(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~EEPGenerator();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "EEPGenerator"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Genereates entry-/exit point textures for the given image and camera."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_sourceImageID;    ///< image ID for input image
        DataNameProperty p_geometryID;       ///< ID for input geometry
        DataNameProperty p_geometryImageId;  ///< image ID for the optional rendered geometry to integrate into the EEP
        DataNameProperty p_camera;           ///< input camra
        DataNameProperty p_entryImageID;     ///< image ID for output entry points image
        DataNameProperty p_exitImageID;      ///< image ID for output exit points image


        BoolProperty p_enableMirror;         ///< Enable Virtual Mirror Feature
        DataNameProperty p_mirrorID;         ///< ID for input mirror geometry


    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * \see RaycastingProcessor::generateHeader()
         * \return  "#define APPLY_MASK 1" if \a _applyMask is set to true.
         */
        virtual std::string generateHeader() const;

        cgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };
}

#endif // EEPGENERATOR_H__
