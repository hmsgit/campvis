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

#include "usfanrenderer.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/geometrydatafactory.h"

namespace campvis {

    const std::string UsFanRenderer::loggerCat_ = "CAMPVis.modules.cudaconfidencemaps.UsFanRenderer";

    UsFanRenderer::UsFanRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImageID", "Input Image ID", "us.input", DataNameProperty::READ)
        , p_renderTargetID("RenderTargetID", "Render Target ID", "us.output", DataNameProperty::WRITE)
        , p_halfAngle("HalfAngle", "Fan Half Angle", 45.0f, 1.0f, 90.0f)
        , p_innerRadius("InnerRadius", "Fan Inner Radius", 0.2f, 0.0f, 0.99f)
        , p_text("Text", "Text", "Ultrasound Title")
        , p_fontFileName("FontFileName", "Path to the Font File to Use", "", StringProperty::OPEN_FILENAME)
        , p_fontSize("FontSize", "Font Size", 20, 4, 100)
        , _shader(0)
        , _grid(nullptr)
    {
        addProperty(p_inputImage);
        addProperty(p_renderTargetID);
        addProperty(p_halfAngle);
        addProperty(p_innerRadius);

        addProperty(p_text);
        addProperty(p_fontFileName);
        addProperty(p_fontSize);

        p_fontFileName.setValue(ShdrMgr.completePath("/modules/fontrendering/fonts/FreeSans.ttf"));
    }

    UsFanRenderer::~UsFanRenderer() {
    }

    void UsFanRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("modules/cudaconfidencemaps/glsl/usfanrenderer.vert", "modules/cudaconfidencemaps/glsl/usfanrenderer.frag", "");
        // Creates the grid, with the origin at the center of the top edge, with the +y axis representing depth
        _grid = GeometryDataFactory::createGrid(cgt::vec3(-0.5f, 1.0f, 0.0f), cgt::vec3(0.5f, 0.0f, 0.0f),
                                                cgt::vec3(0.0f, 1.0f, 0.0f), cgt::vec3(1.0f, 0.0f, 0.0f),
                                                16, 4);

        // Initialize font rendering
        updateFontAtlas();
    }

    void UsFanRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = nullptr;
        _grid = nullptr;
        _atlas = nullptr;

        VisualizationProcessor::deinit();
    }

    void UsFanRenderer::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation texture(data, p_inputImage.getValue());

        // Check that the needed resources have been initialized. Error should never happen.
        if (_shader == 0 || _grid == 0) {
            LDEBUG("Shader or Grid are not initialized.");
            return;
        }

        // Only display the fan if there exists a valid input texture
        if (texture != nullptr) {
            float halfAngle = cgt::deg2rad(p_halfAngle.getValue());
            float innerRadius = p_innerRadius.getValue();

            // calculate bounding box of the US fan
            cgt::vec3 bbCenter = cgt::vec3(0.0f, cos(halfAngle)*innerRadius/2.0f + 0.5f, 0.0f);
            float bbHeight = 1.0f - cos(halfAngle)*innerRadius;
            float bbWidth = sin(halfAngle) * 2.0f;

            cgt::vec2 viewportSize = cgt::vec2(getEffectiveViewportSize());

            // Flip y axis and add a little border
            cgt::mat4 viewportMatrix = cgt::mat4::createScale(cgt::vec3(1, -1, 1) * 0.95f);

            // Adjust the size of the fan to the size of the viewport 
            if (viewportSize.y / bbHeight * bbWidth > viewportSize.x) {
                viewportMatrix *= cgt::mat4::createScale(cgt::vec3(1.0f, viewportSize.x / viewportSize.y, 1.0f) * (1.0f / bbWidth * 2.0f));
            } else {
                viewportMatrix *= cgt::mat4::createScale(cgt::vec3(viewportSize.y / viewportSize.x, 1.0f, 1.0f) * (1.0f / bbHeight * 2.0f));
            }

            // Move the fan center
            viewportMatrix *= cgt::mat4::createTranslation(-bbCenter);

            _shader->activate();
            cgt::TextureUnit textureUnit;
            textureUnit.activate();
            if (texture != nullptr)
                texture->bind(_shader, textureUnit, "_texture", "_textureParams");

            _shader->setUniform("_projectionMatrix", viewportMatrix);
            _shader->setUniform("halfAngle", halfAngle);
            _shader->setUniform("innerRadius", innerRadius);

            // Set wrapping mode to mirror, in order to avoid artefacts at the edges of the fan
            const_cast<cgt::Texture*>(texture->getTexture())->setWrapping(cgt::Texture::MIRRORED_REPEAT);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();

            glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            _grid->render(GL_TRIANGLE_STRIP);
            _shader->deactivate();

            // Render text (if any)
            if (_atlas != nullptr) {
                const cgt::mat4 transformation = cgt::mat4::createTranslation(cgt::vec3(-1.f, -1.f, 0.f))
                                               * cgt::mat4::createScale(cgt::vec3(2.f / _viewportSizeProperty->getValue().x, 2.f / _viewportSizeProperty->getValue().y, 1.f));
                cgt::vec2 pos(32.0f, static_cast<float>(_viewportSizeProperty->getValue().y - 32.0f));
                _atlas->renderText(p_text.getValue(), pos, cgt::vec4(1.0f), transformation);
            }

            LGL_ERROR;

            data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
        }
    }

    void UsFanRenderer::updateFontAtlas() {
        _atlas = nullptr;
        try {
            _atlas = std::unique_ptr<fontrendering::FontAtlas>(new fontrendering::FontAtlas(p_fontFileName.getValue(), p_fontSize.getValue()));
        }
        catch (...) {
            LERROR("Could not create FontAtlas, UsFanRenderer will not display any text.");                
        }
    }
}
