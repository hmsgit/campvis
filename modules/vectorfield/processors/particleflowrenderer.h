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

#ifndef PARTICLEFLOWRENDERER_H__
#define PARTICLEFLOWRENDERER_H__

#include <random>
#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"

#include "core/properties/allproperties.h"

#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/geometrydata.h"

namespace tgt {
    class BufferObject;
    class Shader;
    class VertexArrayObject;
}

namespace campvis {
    /**
     * Renders a vector field with arrows.
     */
    class ParticleFlowRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new ParticleFlowRenderer Processor
         **/
        ParticleFlowRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~ParticleFlowRenderer();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ParticleFlowRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Visualization of a flow field using a particle system."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::deinit()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();
        
        ButtonProperty p_resetButton;
        ButtonProperty p_stepButton;
        IntProperty p_stepInt;

        DataNameProperty p_inputVectors;        ///< ID for input vector image

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

        CameraProperty p_camera;                                    ///< camera
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
        void renderVectorArrow(const GenericImageRepresentationLocal<float, 3>* vectors, const tgt::vec3& position, float scale);


        void initializeTransformFeedbackBuffers(const GenericImageRepresentationLocal<float, 3>* vectors);

        float generateRandomFloat(float max = 1.f);

        GeometryData* _arrowGeometry;		///< Geometry for arrow rendering
        tgt::Shader* _shader;               ///< Shader for arrow rendering

        GLuint _feedback[2];
        float _currentTime;
        GLuint _drawBuffer;
        GLuint _numParticles;

        tgt::BufferObject* _positionBufferA;        ///< Buffers storing current position
        tgt::BufferObject* _positionBufferB;        ///< Buffers storing current position
        tgt::BufferObject* _velocityBufferA;        ///< Buffers storing current velocity
        tgt::BufferObject* _velocityBufferB;        ///< Buffers storing current velocity
        tgt::BufferObject* _startTimeBufferA;       ///< Buffers storing the start time
        tgt::BufferObject* _startTimeBufferB;       ///< Buffers storing the start time
        tgt::BufferObject* _initialPositionBuffer;  ///< Buffers storing the start time

        tgt::VertexArrayObject* _vaoA;
        tgt::VertexArrayObject* _vaoB;

        std::default_random_engine _generator;

        static const std::string loggerCat_;
    };

}

#endif // PARTICLEFLOWRENDERER_H__
