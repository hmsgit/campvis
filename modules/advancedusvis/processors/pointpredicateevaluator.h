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

#ifndef POINTPREDICATEEVALUATOR_H__
#define POINTPREDICATEEVALUATOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/advancedusvis/properties/pointpredicatehistogramproperty.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Evaluates the Point Predicate Histogram and stores the results in a bitmask image.
     */
    class CAMPVIS_MODULES_API PointPredicateEvaluator : public VisualizationProcessor {
    public:
        /**
         * Constructs a new PointPredicateEvaluator Processor
         **/
        PointPredicateEvaluator(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~PointPredicateEvaluator();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "PointPredicateEvaluator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Evaluates the Point Predicate Histogram and stores the results in a bitmask image."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;          ///< Input image
        DataNameProperty p_inputLabels;         ///< Input label image
        DataNameProperty p_inputSnr;            ///< Input SNR image
        DataNameProperty p_inputVesselness;     ///< Input Vesselness image
        DataNameProperty p_inputConfidence;     ///< Input confidence image
        DataNameProperty p_camera;              ///< Input camera (for silhouette view predicate)
            
        DataNameProperty p_outputImage;         ///< Output bitmask image

        PointPredicateHistogramProperty p_histogram;    ///< Point Predicate Histogram to evaluate

    protected:
        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);
        /// adapts the range of the p_sliceNumber property to the image
        virtual void updateProperties(DataContainer dataContainer);
        /// \see AbstractProcessor::updateShader()
        virtual void updateShader();

        /// Callback called when predicate histogram configuration has changed, so that the GLSL header needs to be recompiled.
        void onHistogramHeaderChanged();

        /// Generates the GLSL header
        std::string generateHeader() const;

        cgt::Shader* _shader;                           ///< Shader for slice rendering

    private:
        static const std::string loggerCat_;
    };

}

#endif // POINTPREDICATEEVALUATOR_H__
