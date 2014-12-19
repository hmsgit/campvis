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

#include "textrenderer.h"

#include "core/datastructures/renderdata.h"
#include "modules/fontrendering/tools/fontatlas.h"

namespace campvis {
namespace fontrendering {

    const std::string TextRenderer::loggerCat_ = "CAMPVis.modules.io.TextRenderer";

    TextRenderer::TextRenderer(IVec2Property* viewportSizeProp) 
        : VisualizationProcessor(viewportSizeProp)
        , p_text("Text", "Text", "The Quick Brown Fox Jumps Over The Lazy Dog")
        , p_position("Position", "Position (in Viewport Coordinates)", cgt::ivec2(50), cgt::ivec2(-1000), cgt::ivec2(1000))
        , p_fontSize("FontSize", "Font Size", 20, 4, 100)
        , p_color("Color", "Font Color", cgt::vec4(1.f), cgt::vec4(0.f), cgt::vec4(1.f))
        , p_outputImage("LightId", "Light Name/ID", "lightsource", DataNameProperty::WRITE)
        , _atlas(nullptr)
    {
        addProperty(p_text);
        addProperty(p_position);
        addProperty(p_fontSize, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_color);
        addProperty(p_outputImage);
    }

    TextRenderer::~TextRenderer() {

    }

    void TextRenderer::init() {
        VisualizationProcessor::init();
        _atlas = new FontAtlas("C:\\temp\\FreeSans.ttf", 20);
    }

    void TextRenderer::denit() {
        delete _atlas;
        VisualizationProcessor::deinit();
    }
    void TextRenderer::updateResult(DataContainer& data) {
        FramebufferActivationGuard fag(this);
        createAndAttachColorTexture();
        createAndAttachDepthTexture();

        _atlas->renderText(p_text.getValue(), p_position.getValue(), p_color.getValue(), cgt::vec2(1.f) / cgt::vec2(_viewportSizeProperty->getValue()));

        data.addData(p_outputImage.getValue(), new RenderData(_fbo));
    }

    void TextRenderer::updateProperties(DataContainer& dataContainer) {
        delete _atlas;
        _atlas = nullptr;
        _atlas = new FontAtlas("C:\\temp\\FreeSans.ttf", p_fontSize.getValue());
    }


}
}