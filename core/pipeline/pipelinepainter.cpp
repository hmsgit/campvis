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

#include "pipelinepainter.h"

#include "cgt/assert.h"
#include "cgt/camera.h"
#include "cgt/glcontextmanager.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string PipelinePainter::loggerCat_ = "CAMPVis.core.PipelinePainter";

    PipelinePainter::PipelinePainter(cgt::GLCanvas* canvas, AbstractPipeline* pipeline)
        : cgt::Painter(canvas)
        , _pipeline(pipeline)
        , _copyShader(nullptr)
        , _errorTexture(nullptr)
    {
        cgtAssert(pipeline != nullptr, "The given pipeline must not be 0!");
    }

    void PipelinePainter::paint() {
        if (getCanvas() == nullptr)
            return;

        if (_copyShader == nullptr) {
            LERROR("Shader not initialized!");
            return;
        }

        const cgt::ivec2& size = getCanvas()->getSize();
        glViewport(0, 0, size.x, size.y);

        // try get Data
        DataHandle dh = _pipeline->getDataContainer().getData(_pipeline->getRenderTargetID());
        const RenderData* rd = nullptr;
        const ImageRepresentationGL* repGL = nullptr;
        if (dh.getData() != nullptr) {
            rd = dynamic_cast<const RenderData*>(dh.getData());
            if (const ImageData* id = dynamic_cast<const ImageData*>(dh.getData()))
                repGL = id->getRepresentation<ImageRepresentationGL>();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate the shader
        _copyShader->activate();
        cgt::Shader::IgnoreUniformLocationErrorGuard guard(_copyShader);

        // render whatever there is to render
        if (rd != nullptr || (repGL != nullptr && repGL->getDimensionality() == 2)) {
            _copyShader->setUniform("_viewMatrix", cgt::mat4::identity);

            // bind input textures
            cgt::TextureUnit colorUnit;
            if (rd)
                rd->bindColorTexture(_copyShader, colorUnit);
            else if (repGL)
                repGL->bind(_copyShader, colorUnit);

            // execute the shader
            QuadRdr.renderQuad();
        }
        // if there is nothing to render, render the error texture
        else if (_errorTexture != nullptr) {
            float ratioRatio = static_cast<float>(size.y) / size.x;
            cgt::mat4 viewMatrix = (ratioRatio > 1) ? cgt::mat4::createScale(cgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : cgt::mat4::createScale(cgt::vec3(ratioRatio, 1.f, 1.f));
            _copyShader->setUniform("_viewMatrix", viewMatrix);

            // bind input textures
            cgt::TextureUnit colorUnit;
            colorUnit.activate();
            _errorTexture->bind();
            _copyShader->setUniform("_colorTexture", colorUnit.getUnitNumber());

            // execute the shader
            QuadRdr.renderQuad();
        }
        else {
            LERROR("Nothing to render but could not load error texture either.");
        }

        _copyShader->deactivate();
        LGL_ERROR;

        _pipeline->paint();

        getCanvas()->swap();
    }

    void PipelinePainter::sizeChanged(const cgt::ivec2& size) {
        _pipeline->setRenderTargetSize(size);
    }

    void PipelinePainter::init() {
        try {
            _copyShader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "");
        }
        catch (cgt::Exception& e) {
            LFATAL("Encountered cgt::Exception: " << e.what());
        }
    }

    void PipelinePainter::deinit() {
        ShdrMgr.dispose(_copyShader);

        if (getCanvas() != nullptr) {
            if (getCanvas()->getEventHandler() != nullptr)
                getCanvas()->getEventHandler()->removeEventListener(_pipeline);
            _pipeline = nullptr;
        }
    }

    void PipelinePainter::repaint() {
        _pipeline->setPipelineDirty();
    }

    void PipelinePainter::setErrorTexture(cgt::Texture* texture) {
        _errorTexture = texture;
    }

    void PipelinePainter::setCanvas(cgt::GLCanvas* canvas) {
        if (getCanvas() != nullptr) {
            if (getCanvas()->getEventHandler() != nullptr)
                getCanvas()->getEventHandler()->removeEventListener(_pipeline);
        }

        cgt::Painter::setCanvas(canvas);

        if (getCanvas() != nullptr) {
            _pipeline->setRenderTargetSize(getCanvas()->getSize());
            if (getCanvas()->getEventHandler() != nullptr)
                getCanvas()->getEventHandler()->addEventListenerToFront(_pipeline);
        }
    }

}
