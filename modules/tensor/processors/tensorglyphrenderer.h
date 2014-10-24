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

#ifndef TENSORGLYPHRENDERER_H__
#define TENSORGLYPHRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"

#include "core/properties/allproperties.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/geometrydata.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Renders axis-aligned slices with tensor glyphs.
     */
    class TensorGlyphRenderer : public VisualizationProcessor {
    public:
        /// Glyph type to render
        enum GlyphType {
            ELLIPSOID,
            CUBOID,
            MULTI
        };

        /// Slice orientation
        enum SliceOrientation {
            XY_PLANE = 0,
            XZ_PLANE = 1,
            YZ_PLANE = 2
        };

        /**
         * Constructs a new TensorGlyphRenderer Processor
         **/
        TensorGlyphRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~TensorGlyphRenderer();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "TensorGlyphRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders axis-aligned slices with tensor glyphs."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::deinit()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        DataNameProperty p_inputEigenvalues;    ///< ID for input eigenvalues
        DataNameProperty p_inputEigenvectors;   ///< ID for input eigenvectors
        DataNameProperty p_camera;              ///< ID for camera input
        DataNameProperty p_renderOutput;        ///< ID for output rendered image

        GenericOptionProperty<GlyphType> p_glyphType;   ///< Glyph type to render
        FloatProperty p_glyphSize;                      ///< Glyph render size

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

        /**
         * Renders a single tensor glyph at the given position
         * \param   evals       Eigenvalue image
         * \param   evecs       Eigenvector image
         * \param   position    Image position to render in voxel coordinates
         */
        void renderTensorGlyph(const GenericImageRepresentationLocal<float, 3>* evals, const GenericImageRepresentationLocal<float, 9>* evecs, const cgt::vec3& position);

        cgt::Shader* _shader;               ///< Shader for glyph rendering
        GeometryData* _ellipsoidGeometry;   ///< Geometry for ellipsoid rendering
        GeometryData* _cubeGeometry;        ///< Geometry for cuboid rendering

        static const std::string loggerCat_;
    };

}

#endif // TENSORGLYPHRENDERER_H__
