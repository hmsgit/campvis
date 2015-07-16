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

#ifndef VECTORFIELDRENDERER_H__
#define VECTORFIELDRENDERER_H__

#include <string>
#include <memory>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"

#include "core/properties/allproperties.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/geometrydata.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Renders a vector field with arrows.
     */
    class CAMPVIS_MODULES_API VectorFieldRenderer : public VisualizationProcessor {
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
        explicit VectorFieldRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~VectorFieldRenderer();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "VectorFieldRenderer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
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
        
        DataNameProperty p_inputVectors;        ///< ID for input vector image
        DataNameProperty p_camera;              ///< ID for camera input
        DataNameProperty p_renderOutput;        ///< ID for output rendered image

        FloatProperty p_arrowSize;                      ///< Arrow size size
        FloatProperty p_lenThresholdMin;                ///< Threshold minimum
        FloatProperty p_lenThresholdMax;                ///< Threshold maximum

        FloatProperty p_flowProfile1;
        FloatProperty p_flowProfile2;
        FloatProperty p_flowProfile3;
        FloatProperty p_flowProfile4;

        IntProperty p_Time;

        BoolProperty p_enableShading;               ///< Flag whether to enable shading
        DataNameProperty p_lightId;                 ///< Name/ID for the LightSource to use

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

        // for B-Spline evaluation
        float getTemporalFlowScaling(float t, float Ct0, float Ct1, float Ct2, float Ct3);
        float evaluateCubicBSpline(float t);

        /**
         * Renders a single vector arrow
         * \param   vectors     Input vector image
         * \param   position    Position to render
         */
        void renderVectorArrow(const GenericImageRepresentationLocal<float, 3>* vectors, const cgt::vec3& position, float scale);

        std::unique_ptr<GeometryData> _arrowGeometry;   ///< Geometry for arrow rendering
        cgt::Shader* _shader;                           ///< Shader for arrow rendering
        
        static const std::string loggerCat_;
    };
}

#endif // VECTORFIELDRENDERER_H__
