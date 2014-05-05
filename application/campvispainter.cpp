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

#include "campvispainter.h"

#include "tgt/assert.h"
#include "tgt/camera.h"
#include "tgt/quadric.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/qt/qtthreadedcanvas.h"


#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string CampVisPainter::loggerCat_ = "CAMPVis.core.CampVisPainter";

    CampVisPainter::CampVisPainter(tgt::GLCanvas* canvas, AbstractPipeline* pipeline)
        : tgt::Painter(canvas)
        , _pipeline(0)
        , _copyShader(0)
    {
        tgtAssert(getCanvas() != 0, "The given canvas must not be 0!");
        _dirty = true;
        setPipeline(pipeline);
    }

    CampVisPainter::~CampVisPainter() {

    }

    void CampVisPainter::paint() {
        if (getCanvas() == 0)
            return;

        const tgt::ivec2& size = getCanvas()->getSize();
        glViewport(0, 0, size.x, size.y);

        // try get Data
        ScopedTypedData<RenderData> rd(_pipeline->getDataContainer(), _pipeline->getRenderTargetID());
        ImageRepresentationGL::ScopedRepresentation repGL(_pipeline->getDataContainer(), _pipeline->getRenderTargetID());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (rd != 0 || (repGL != 0 && repGL->getDimensionality() == 2)) {
            // activate shader
            _copyShader->activate();
            _copyShader->setIgnoreUniformLocationError(true);
            _copyShader->setUniform("_viewportSize", size);
            _copyShader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(size));
            _copyShader->setIgnoreUniformLocationError(false);

            // bind input textures
            tgt::TextureUnit colorUnit;
            if (rd)
                rd->bindColorTexture(_copyShader, colorUnit);
            else if (repGL)
                repGL->bind(_copyShader, colorUnit);
            LGL_ERROR;

            // execute the shader
            _copyShader->setAttributeLocation(0, "in_Position");
            _copyShader->setAttributeLocation(1, "in_TexCoords");
            QuadRdr.renderQuad();
            _copyShader->deactivate();
            LGL_ERROR;
        }
        else {
            // TODO: render some nifty error texture
            //       so long, we do some dummy rendering
            tgt::Camera c(tgt::vec3(0.f,0.f,2.f)); 
            c.look();  
            glColor3f(1.f, 0.f, 0.f);  
            tgt::Sphere sphere(.5f, 64, 32);  
            sphere.render();

            /*
            // render error texture
            if (!errorTex_) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                return;
            }
            glClear(GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            errorTex_->bind();
            errorTex_->enable();

            glColor3f(1.f, 1.f, 1.f);
            renderQuad();

            errorTex_->disable();*/
        }
        LGL_ERROR;

        getCanvas()->swap();
    }

    void CampVisPainter::sizeChanged(const tgt::ivec2& size) {
        _pipeline->setRenderTargetSize(size);
    }

    void CampVisPainter::init() {
        try {
            // TODO:    Remove hardcoded paths, and use ShdrMgr.addPath() at some central location
            _copyShader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "");
        }
        catch (tgt::Exception& e) {
            LERRORC("main.cpp", "Encountered tgt::Exception: " << e.what());
        }
    }

    void CampVisPainter::deinit() {
        ShdrMgr.dispose(_copyShader);

        if (_pipeline != 0) {
            _pipeline->s_renderTargetChanged.disconnect(this);
            if (getCanvas()->getEventHandler() != 0)
                getCanvas()->getEventHandler()->removeEventListener(_pipeline);
            _pipeline = 0;
        }
    }

    void CampVisPainter::setPipeline(AbstractPipeline* pipeline) {
        tgtAssert(pipeline != 0, "The given pipeline must not be 0.");
        if (_pipeline != 0) {
            _pipeline->s_renderTargetChanged.disconnect(this);
            if (getCanvas()->getEventHandler() != 0)
                getCanvas()->getEventHandler()->removeEventListener(_pipeline);
        }

        _pipeline = pipeline;
        _pipeline->s_renderTargetChanged.connect(this, &CampVisPainter::onRenderTargetChanged);
        _pipeline->setRenderTargetSize(getCanvas()->getSize());
        if (getCanvas()->getEventHandler() != 0)
            getCanvas()->getEventHandler()->addEventListenerToFront(_pipeline);
    }

    void CampVisPainter::repaint() {
        GLJobProc.enqueueJob(getCanvas(), makeJobOnHeap(this, &CampVisPainter::paint), OpenGLJobProcessor::PaintJob);
    }

    void CampVisPainter::onRenderTargetChanged() {
        repaint();
    }

    void CampVisPainter::setCanvas(tgt::GLCanvas* canvas) {
        tgtAssert(dynamic_cast<tgt::QtThreadedCanvas*>(canvas) != 0, "Canvas must be of type QtThreadedCanvas!");
        Painter::setCanvas(canvas);
    }
}
