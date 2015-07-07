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

#ifndef GLVESSELNESSFILTER_H__
#define GLVESSELNESSFILTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Computes a Vesselness Measure very similar to the one proposed by Frangi.
     */
    class CAMPVIS_MODULES_API GlVesselnessFilter : public VisualizationProcessor {
    public:
        /**
         * Constructs a new GlVesselnessFilter Processor
         **/
        GlVesselnessFilter(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GlVesselnessFilter();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "GlVesselnessFilter"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Computes a Vesselness Measure very similar to the one proposed by Frangi."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;      ///< ID for input volume
        DataNameProperty p_outputImage;     ///< ID for output gradient volume

        Vec2Property p_lod;         ///< Minimum/Maximum LOD for multi-level texture lookup
        FloatProperty p_alpha;      ///< Alpha Parameter
        FloatProperty p_beta;       ///< Beta Parameter
        FloatProperty p_gamma;      ///< Gamma Parameter
        FloatProperty p_theta;      ///< Theta Parameter

        BoolProperty p_halfPrecisionOutput; ///< Flag whether to use only half precision (16 Bit) output texture

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        cgt::Shader* _shader;       ///< Shader for gradient volume generation

        static const std::string loggerCat_;
    };
}

#endif // GLVESSELNESSFILTER_H__
