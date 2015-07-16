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

#ifndef SCANLINEDISTRIBUTIONCOMPUTATION_H__
#define SCANLINEDISTRIBUTIONCOMPUTATION_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"

#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Computes the intensity distribution profile for each voxel along the scanline.
     */
    class CAMPVIS_MODULES_API ScanlineDistributionComputation : public VisualizationProcessor {
    public:
        /**
         * Constructs a new ScanlineDistributionComputation Processor
         **/
        explicit ScanlineDistributionComputation(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~ScanlineDistributionComputation();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// To be used in ProcessorFactory static methods
        static const std::string getId() { return "ScanlineDistributionComputation"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Computes the intensity distribution profile for each voxel along the scanline."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;
        DataNameProperty p_outputImage;

        Vec3Property p_scanningDirection;
        FloatProperty p_threshold;
        FloatProperty p_stepSize;

    protected:
        /// \see AbstractProcessor::process()
        virtual void updateResult(DataContainer& dataContainer);
        /// adapts the range of the p_sliceNumber property to the image
        virtual void updateProperties(DataContainer dataContainer);

        std::string generateHeader() const;

        cgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // SCANLINEDISTRIBUTIONCOMPUTATION_H__
