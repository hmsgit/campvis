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

#ifndef SLICERENDERPROCESSOR_H__
#define SLICERENDERPROCESSOR_H__

#include <string>

#include "cgt/event/eventlistener.h"

#include "core/datastructures/imagerepresentationgl.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class FaceGeometry;
    class ImageData;

    /**
     * Base class for rendering an axis-aligned slice of a 3D image into a 2D viewport.
     * Provides necessary properties, coordinate transforms, interaction handlers, as well as a
     * callback for a scribbling feature.
     * 
     * The single thing to be implemented by subclasses is the way the actual image is rendered.
     */
    class CAMPVIS_CORE_API SliceRenderProcessor : public VisualizationProcessor, public tgt::EventListener {
    public:
        /// Slice Orientation to render
        enum SliceOrientation {
            XY_PLANE = 0,
            XZ_PLANE = 1,
            YZ_PLANE = 2
        };

        /**
         * Constructs a new SliceRenderProcessor Processor
         * 
         * \param   viewportSizeProp            Pointer to the parent pipeline's render target size property.
         * \param   fragmentShaderFileName      Filename for the fragment shader being automatically loaded.
         * \param   customGlslVersion           Custom GLSL version to pass to shader (Optional).
         **/
        SliceRenderProcessor(IVec2Property* viewportSizeProp, const std::string& fragmentShaderFileName, const std::string& customGlslVersion = "");

        /**
         * Destructor
         **/
        virtual ~SliceRenderProcessor();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see tgt::EventListener::onEvent()
        virtual void onEvent(tgt::Event* e);

        /// Signal emitted when a scribble was painted, parameter gives the position in image coordinates.
        sigslot::signal1<tgt::vec3> s_scribblePainted;

        DataNameProperty p_sourceImageID;                   ///< image ID for input image
        DataNameProperty p_geometryID;                      ///< ID for input geometry
        DataNameProperty p_targetImageID;                   ///< image ID for output image

        /// orientation of the slice to extract
        GenericOptionProperty<SliceOrientation> p_sliceOrientation;

        IntProperty p_xSliceNumber;                         ///< number of the slice in X direction
        Vec4Property p_xSliceColor;                         ///< color for x marker
        IntProperty p_ySliceNumber;                         ///< number of the slice in Y direction
        Vec4Property p_ySliceColor;                         ///< color for y marker
        IntProperty p_zSliceNumber;                         ///< number of the slice in Z direction
        Vec4Property p_zSliceColor;                         ///< color for z marker
        BoolProperty p_renderCrosshair;                     ///< Flag whether to render the crosshair or not

        BoolProperty p_fitToWindow;                         ///< Flag whether fit image to window or use scaling and offset
        FloatProperty p_scalingFactor;                      ///< Image scaling factor
        IVec2Property p_offset;                             ///< Image offset

        GenericOptionProperty<GLenum> p_geometryRenderMode; ///< Render mode for the geometry
        FloatProperty p_geometryRenderSize;                 ///< Size of rendered elements

    protected:
        /**
         * Pure virtual method to be implemented by subclasses.
         * This method is supposed to implement the actual rendering of the slice, e.g. setup
         * and apply transfer function. You may want to call renderCrosshair() or renderGeometry() 
         * yourself, if needed.
         * \param   dataContainer   DataContainer of calling pipeline
         * \param   img             Image to render (to avoid multiple redundant DataContainer queries), is guarantueed to not be 0)
         */
        virtual void renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img) = 0;

        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * Renders the crosshair indicating the slice positions into the current OpenGl context.
         * \param   img             Image to render (to avoid multiple redundant DataContainer queries), is guarantueed to not be 0)
         */
        virtual void renderCrosshair(const ImageRepresentationGL::ScopedRepresentation& img);

        /**
         * Renders integrated geometry into the current OpenGL context.
         * \param   dataContainer   DataContainer of calling pipeline
         * \param   img             Image to render (to avoid multiple redundant DataContainer queries), is guarantueed to not be 0)
         */
        virtual void renderGeometry(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img);

        /**
         * Setup all the transformation matrices (local members) needed for rendering.
         * Gets called automatically from default updateResult() implementation prior to calling renderImageImpl().
         * \param   img             Image to render (to avoid multiple redundant DataContainer queries), is guarantueed to not be 0)
         */
        virtual void setupMatrices(const ImageRepresentationGL::ScopedRepresentation& img);

        virtual std::string getGlslHeader();

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        DataHandle _currentImage;                       ///< cached DataHandle to shown image (needed for scribbles)
        bool _inScribbleMode;                           ///< Flag whether processor is in scribble mode (i.e. mouse is pressed)

        const std::string _fragmentShaderFilename;      ///< Filename for the fragment shader being automatically loaded.
        const std::string _customGlslVersion;           ///< Custom GLSL version to pass to shader


        tgt::mat4 _texCoordMatrix;                      ///< Transformation matrix applied to texture coordinates
        tgt::mat4 _geometryModelMatrix;                 ///< Transformation to apply to geometry to transform into slice space
        tgt::mat4 _geometryProjectionMatrix;            ///< Projection matrix to apply to geometry rendering
        tgt::mat4 _viewMatrix;                          ///< View matrix applied to rendering (aspect ratio, zoom, shift)

        static const std::string loggerCat_;
    };

}

#endif // SLICERENDERPROCESSOR_H__
