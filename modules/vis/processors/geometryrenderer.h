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

#ifndef GEOMETRYRENDERER_H__
#define GEOMETRYRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class GeometryRenderer : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /// Coloring mode for fragments used during rendering
        enum ColoringMode {
            GEOMETRY_COLOR = 0,     ///< Original color as stored in geometry
            SOLID_COLOR = 1,        ///< Solid color set by property
            TEXTURE_COLOR = 2       ///< Color determined from texture lookup
        };

        /**
         * Constructs a new GeometryRenderer Processor
         **/
        GeometryRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GeometryRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GeometryRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders Geometry."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_geometryID;              ///< ID for input geometry
        DataNameProperty p_textureID;               ///< ID for input texture (optional)
        DataNameProperty p_renderTargetID;          ///< image ID for output image
        CameraProperty p_camera;

        GenericOptionProperty<GLenum> p_renderMode;         ///< Render mode for the geometry
        GenericOptionProperty<ColoringMode> p_coloringMode; ///< Coloring mode for fragments used during rendering

        Vec4Property p_solidColor;                  ///< rendering color

        FloatProperty p_pointSize;                  ///< Point Size when rendering points
        FloatProperty p_lineWidth;                  ///< Line Width when rendering lines

        BoolProperty p_showWireframe;               ///< Show wire frame
        Vec4Property p_wireframeColor;              ///< Wireframe color
        

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * Generates the GLSL header.
         */
        std::string generateGlslHeader(bool hasGeometryShader) const;

        tgt::Shader* _pointShader;
        tgt::Shader* _meshShader;

    private:
        static const std::string loggerCat_;
    };

}

#endif // GEOMETRYRENDERER_H__
