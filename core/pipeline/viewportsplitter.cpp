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

#include "viewportsplitter.h"

#include "cgt/framebufferobject.h"
#include "cgt/textureunit.h"
#include "cgt/event/mouseevent.h"
#include "cgt/event/keyevent.h"

#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

namespace campvis {

    const std::string ViewportSplitter::loggerCat_ = "CAMPVis.core.datastructures.ViewportSplitter";

// ================================================================================================

    ViewportSplitter::ViewportSplitter(size_t numSubViews, SplitMode splitMode, IVec2Property* viewportSizeProp)
        : p_outputImageId("OutputImageId", "Output Image ID", "", DataNameProperty::WRITE)
        , p_subViewViewportSize("ElementViewportSize", "Element Viewport Size", cgt::ivec2(128), cgt::ivec2(1), cgt::ivec2(2048))
        , _numSubViews(numSubViews)
        , _splitMode(splitMode)
        , _mousePressed(false)
        , _viewIndexOfEvent(0)
        , _fbo(nullptr)
        , p_viewportSizeProperty(viewportSizeProp)
        , _copyShader(nullptr)
    {
        p_viewportSizeProperty->s_changed.connect(this, &ViewportSplitter::onViewportSizePropertyChanged);
        p_inputImageIds.resize(_numSubViews, nullptr);
    }

    ViewportSplitter::~ViewportSplitter() {
    }

    void ViewportSplitter::init() {
        _quad = GeometryDataFactory::createQuad(cgt::vec3(0.f), cgt::vec3(1.f), cgt::vec3(0.f), cgt::vec3(1.f));
        _fbo = new cgt::FramebufferObject();

        _copyShader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "");
        _copyShader->setAttributeLocation(0, "in_Position");
        _copyShader->setAttributeLocation(1, "in_TexCoord");
    }

    void ViewportSplitter::deinit() {
        delete _fbo;
        delete _quad;
        ShdrMgr.dispose(_copyShader);
    }

    void ViewportSplitter::setInputImageIdProperty(size_t index, DataNameProperty* prop) {
        cgtAssert(index < _numSubViews, "Index out of bounds");
        p_inputImageIds[index] = prop;
    }

    void ViewportSplitter::onEvent(cgt::Event* e) {
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);
            cgt::ivec2 position(me->x(), me->y());

            if (! _mousePressed) {
                if (_splitMode == HORIZONTAL)
                    _viewIndexOfEvent = std::min(size_t(position.x / p_subViewViewportSize.getValue().x), _numSubViews);
                else if (_splitMode == VERTICAL)
                    _viewIndexOfEvent = std::min(size_t(position.y / p_subViewViewportSize.getValue().y), _numSubViews);
            }

            if (me->action() == cgt::MouseEvent::PRESSED)
                _mousePressed = true;
            else if (me->action() == cgt::MouseEvent::RELEASED)
                _mousePressed = false;


            // compute adjusted mouse event
            if (_splitMode == HORIZONTAL)
                position.x -= int(_viewIndexOfEvent) * p_subViewViewportSize.getValue().x;
            else if (_splitMode == VERTICAL)
                position.y -= int(_viewIndexOfEvent) * p_subViewViewportSize.getValue().y;
            cgt::MouseEvent adjustedMe(position.x, position.y, me->action(), me->modifiers(), me->button(), p_subViewViewportSize.getValue());

            // trigger signal, this HAS to be done synchroneously
            s_onEvent.triggerSignal(_viewIndexOfEvent, &adjustedMe);
        }
        else {
            // trigger signal, this HAS to be done synchroneously
            s_onEvent.triggerSignal(_viewIndexOfEvent, e);
        }
    }

    void ViewportSplitter::render(DataContainer& dataContainer) {
        cgt::vec2 vps(p_viewportSizeProperty->getValue());
        cgt::vec2 evps(p_subViewViewportSize.getValue());

        cgt::TextureUnit rtUnit, colorUnit, depthUnit;
        rtUnit.activate();
        cgt::Texture* tex = new cgt::Texture(GL_TEXTURE_2D, cgt::ivec3(p_viewportSizeProperty->getValue(), 1), GL_RGBA8, cgt::Texture::LINEAR);
        tex->setWrapping(cgt::Texture::CLAMP_TO_EDGE);

        _fbo->activate();
        _fbo->attachTexture(tex, GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, static_cast<GLsizei>(vps.x), static_cast<GLsizei>(vps.y));

        _copyShader->activate();
        _copyShader->setUniform("_projectionMatrix", cgt::mat4::createOrtho(0, vps.x, vps.y, 0, -1, 1));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < _numSubViews; ++i) {
            if (p_inputImageIds[i] != nullptr) {
                ScopedTypedData<RenderData> rd(dataContainer, p_inputImageIds[i]->getValue());
                if (rd != nullptr) {
                    rd->bind(_copyShader, colorUnit, depthUnit);

                    _copyShader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(evps.x, evps.y, .5f)));
                    if (_splitMode == HORIZONTAL)
                        _copyShader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(float(i) * evps.x, 0.f, 0.f)));
                    else if (_splitMode == VERTICAL)
                        _copyShader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(0.f, float(_numSubViews - i - 1) * evps.y, 0.f)));

                    _quad->render(GL_TRIANGLE_FAN);
                }
            }
        }

        _copyShader->deactivate();
        dataContainer.addData(p_outputImageId.getValue(), new RenderData(_fbo));

        _fbo->detachAll();
        _fbo->deactivate();
    }

    void ViewportSplitter::onViewportSizePropertyChanged(const AbstractProperty* prop) {
        cgtAssert(prop == p_viewportSizeProperty, "Wrong property in signal, this should not happen!");

        const cgt::ivec2& vps = p_viewportSizeProperty->getValue();
        if (_splitMode == HORIZONTAL) {
            p_subViewViewportSize.setValue(cgt::ivec2(vps.x / int(_numSubViews), vps.y));
        }
        else if (_splitMode == VERTICAL) {
            p_subViewViewportSize.setValue(cgt::ivec2(vps.x, vps.y / int(_numSubViews)));
        }
    }

}
