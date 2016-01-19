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

#ifndef CONFIDENCEMAPGENERATOR_H__
#define CONFIDENCEMAPGENERATOR_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Creates Confidence Maps for Ultrasound Images.
     * TODO: Clean up pre-MICCAI mess!
     */
    class CAMPVIS_MODULES_API ConfidenceMapGenerator : public AbstractProcessor {
    public:
        /**
         * Constructs a new ConfidenceMapGenerator Processor
         **/
        ConfidenceMapGenerator();

        /**
         * Destructor
         **/
        virtual ~ConfidenceMapGenerator();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ConfidenceMapGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates Confidence Maps for Ultrasound Images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_sourceImageID;   ///< ID for input image
        DataNameProperty p_targetImageID;   ///< ID for output confidence map image

        FloatProperty p_alpha;                          ///< Alpha Parameter
        FloatProperty p_beta;                           ///< Beta Parameter
        FloatProperty p_gamma;                          ///< Gamma Parameter
        BoolProperty p_normalizeValues;                 ///< Flag whether to normalize the values before computation
        GenericOptionProperty<std::string> p_solver;    ///< Solver to use
        IntProperty p_numSteps;

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // CONFIDENCEMAPGENERATOR_H__
