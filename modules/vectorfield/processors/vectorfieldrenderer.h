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

#ifndef VECTORFIELDRENDERER_H__
#define VECTORFIELDRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"

#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/geometrydata.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Renders a vector field with arrows.
     */
    class VectorFieldRenderer : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /// Slice orientation
        enum SliceOrientation {
            XY_PLANE = 0,
            XZ_PLANE = 1,
            YZ_PLANE = 2,
			XYZ_VOLUME = 3
        };

        /**
         * Constructs a new VectorFieldRenderer Processor
         **/
        VectorFieldRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~VectorFieldRenderer();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VectorFieldRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders vector fields with arrows"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Oliver Zettinig <oliver.zettinig@tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::deinit()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();
        
		DataNameProperty p_inputVectorX;   ///< ID for input vector image (X-component)
		DataNameProperty p_inputVectorY;   ///< ID for input vector image (Y-component)
		DataNameProperty p_inputVectorZ;   ///< ID for input vector image (Z-component)

        DataNameProperty p_renderOutput;        ///< ID for output rendered image

        FloatProperty p_arrowSize;                      ///< Arrow size size
		FloatProperty p_lenThresholdMin;                ///< Threshold minimum
		FloatProperty p_lenThresholdMax;                ///< Threshold maximum

        CameraProperty p_camera;                                    ///< camera
        GenericOptionProperty<SliceOrientation> p_sliceOrientation; ///< orientation of the slice to extract
        IntProperty p_sliceNumber;                                  ///< slice number

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        std::string generateGlslHeader() const;

        /**
         * Renders a single vector arrow
		 * \param   vectorX     Input vector image (X component)
		 * \param   vectorY     Input vector image (Y component)
		 * \param   vectorZ     Input vector image (Z component)
         * \param   position    Position to render
         */
        void renderVectorArrow(const GenericImageRepresentationLocal<float, 1>* vectorX,
			const GenericImageRepresentationLocal<float, 1>* vectorY,
			const GenericImageRepresentationLocal<float, 1>* vectorZ,
			const tgt::vec3& position);

        tgt::Shader* _shader;               ///< Shader for arrow rendering
        GeometryData* _arrowGeometry;		///< Geometry for arrow rendering

        static const std::string loggerCat_;
    };

}

#endif // VECTORFIELDRENDERER_H__
