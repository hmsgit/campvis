// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef GLMORPHOLOGYFILTER_H__
#define GLMORPHOLOGYFILTER_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Creates the gradient volume for the given intensity volume using OpenGL.
     */
    class GlMorphologyFilter : public VisualizationProcessor {
    public:
        /**
         * Constructs a new GlMorphologyFilter Processor
         **/
        GlMorphologyFilter(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GlMorphologyFilter();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GlMorphologyFilter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates the gradient volume for the given intensity volume using OpenGL."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputImage;      ///< ID for input volume
        DataNameProperty p_outputImage;     ///< ID for output gradient volume

        StringProperty p_filterOperation;   ///< String-encoded filter operation to apply

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        /**
         * Applys the morphology filter \a filter to \a inputImage.
         * \param   inputTexture    Input image for the filter
         * \param   filter          Filter to apply (should be _erosionFilter or _dilationFilter or compatible)
         * \return  An OpenGL texture with the filtered image
         */
        tgt::Texture* applyFilter(const tgt::Texture* inputTexture, tgt::Shader* filter) const;


        tgt::Shader* _erosionFilter;        ///< Shader for performing erosion filter
        tgt::Shader* _dilationFilter;       ///< Shader for performing dilation filter

        static const std::string loggerCat_;
    };

}

#endif // GLMORPHOLOGYFILTER_H__
