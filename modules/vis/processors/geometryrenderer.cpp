// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "geometryrenderer.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    const std::string GeometryRenderer::loggerCat_ = "CAMPVis.modules.vis.GeometryRenderer";

    GeometryRenderer::GeometryRenderer(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _geometryID("geometryID", "Input Geometry ID", "gr.input", DataNameProperty::READ)
        , _renderTargetID("_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , _camera("camera", "Camera")
        , _color("color", "Rendering Color", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _shader(0)
    {
        addProperty(&_geometryID);
        addProperty(&_renderTargetID);
        addProperty(&_camera);
        addProperty(&_color);
    }

    GeometryRenderer::~GeometryRenderer() {

    }

    void GeometryRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/geometryrenderer.frag", "", false);
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
        }
    }

    void GeometryRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void GeometryRenderer::process(DataContainer& data) {
        DataContainer::ScopedTypedData<GeometryData> proxyGeometry(data, _geometryID.getValue());

        if (proxyGeometry != 0 && _shader != 0) {
            // set modelview and projection matrices
            glPushAttrib(GL_ALL_ATTRIB_BITS);

            _shader->activate();
            _shader->setUniform("_projectionMatrix", _camera.getValue().getProjectionMatrix());
            _shader->setUniform("_viewMatrix", _camera.getValue().getViewMatrix());
            _shader->setUniform("_color", _color.getValue());

            // create entry points texture
            ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1), GL_RGBA16);
            rt->activate();

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            proxyGeometry->render();

            rt->deactivate();
            _shader->deactivate();
            glPopAttrib();
            LGL_ERROR;

            data.addData(_renderTargetID.getValue(), rt);
            _renderTargetID.issueWrite();
        }
        else {
            LERROR("No suitable input geometry found.");
        }

        _invalidationLevel.setValid();
    }

}