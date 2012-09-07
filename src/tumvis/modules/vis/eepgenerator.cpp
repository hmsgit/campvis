// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "eepgenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/meshgeometry.h"
#include "core/pipeline/processordecoratormasking.h"

namespace TUMVis {
    const std::string EEPGenerator::loggerCat_ = "TUMVis.modules.vis.EEPGenerator";

    EEPGenerator::EEPGenerator(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , _geometryID("geometryID", "Input Geometry ID", "proxygeometry", DataNameProperty::READ)
        , _mirrorID("mirrorID", "Input Mirror ID", "mirror", DataNameProperty::READ)
        , _entryImageID("entryImageID", "Output Entry Points Image", "eep.entry", DataNameProperty::WRITE)
        , _exitImageID("exitImageID", "Output Exit Points Image", "eep.exit", DataNameProperty::WRITE)
        , _camera("camera", "Camera")
        , _enableMirror("enableMirror", "Enable Virtual Mirror Feature", false)
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorMasking());

        addProperty(&_sourceImageID);
        addProperty(&_geometryID);
        addProperty(&_mirrorID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_camera);
        addProperty(&_enableMirror);

        decoratePropertyCollection(this);
    }

    EEPGenerator::~EEPGenerator() {

    }

    void EEPGenerator::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/eepgenerator.frag", "", false);
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
            _shader->setAttributeLocation(1, "in_TexCoord");
        }
    }

    void EEPGenerator::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void EEPGenerator::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<MeshGeometry> proxyGeometry(data, _geometryID.getValue());

        if (img != 0 && proxyGeometry != 0 && _shader != 0) {
            if (img->getDimensionality() == 3) {
                if (_invalidationLevel.isInvalidShader()) {
                    _shader->setHeaders(generateHeader());
                    _shader->rebuild();
                }

                tgt::Bounds volumeExtent = img->getWorldBounds();
                tgt::Bounds textureBounds(tgt::vec3(0.f), tgt::vec3(1.f));

                // clip proxy geometry against near-plane to support camera in volume
                // FIXME:   In some cases, the near plane is not rendered correctly...
                float nearPlaneDistToOrigin = tgt::dot(_camera.getValue().getPosition(), -_camera.getValue().getLook()) - _camera.getValue().getNearDist() - .001f;
                MeshGeometry clipped = proxyGeometry->clipAgainstPlane(nearPlaneDistToOrigin, -_camera.getValue().getLook(), true, 0.02f);

                // start render setup
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                _shader->activate();

                // setup virtual mirror if necessary
                tgt::mat4 mirrorMatrix = tgt::mat4::identity;
                if (_enableMirror.getValue()) {
                    DataContainer::ScopedTypedData<FaceGeometry> mirrorGeometry(data, _mirrorID.getValue());
                    if (mirrorGeometry && mirrorGeometry->size() > 0) {
                        const tgt::vec3& p = mirrorGeometry->getVertices()[0];
                        tgt::vec3 n = tgt::normalize(tgt::cross(mirrorGeometry->getVertices()[1] - mirrorGeometry->getVertices()[0], mirrorGeometry->getVertices()[2] - mirrorGeometry->getVertices()[0]));
                        float k = tgt::dot(p, n);

                        // mirror matrix sponsored by:
                        // Jiang 
                        mirrorMatrix = tgt::transpose(tgt::mat4(
                            1 - 2*n.x*n.x, -2*n.y*n.x   , -2*n.z*n.x   , 0, 
                            -2*n.x*n.y   , 1 - 2*n.y*n.y, -2*n.z*n.y   , 0, 
                            -2*n.x*n.z   , -2*n.y*n.z   , 1 - 2*n.z*n.z, 0, 
                            2*n.x*k      , 2*n.y*k      , 2*n.z*k      , 1));

                        // TODO: double check, whether matrix transpose is necessary
                    }
                    else {
                        LERROR("No suitable virtual mirror geometry found.");
                    }
                }

                decorateRenderProlog(data, _shader);
                
                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
                _shader->setUniform("_modelMatrix", mirrorMatrix);
                _shader->setUniform("_projectionMatrix", _camera.getValue().getProjectionMatrix());
                _shader->setUniform("_viewMatrix", _camera.getValue().getViewMatrix());
                _shader->setIgnoreUniformLocationError(false);

                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);

                // create entry points texture
                ImageDataRenderTarget* entrypoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1), GL_RGBA16);
                entrypoints->activate();

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(_enableMirror.getValue() ? GL_FRONT : GL_BACK);
                clipped.render();

                entrypoints->deactivate();

                // create exit points texture
                ImageDataRenderTarget* exitpoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1), GL_RGBA16);
                exitpoints->activate();

                glDepthFunc(GL_GREATER);
                glClearDepth(0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(_enableMirror.getValue() ? GL_BACK : GL_FRONT);
                clipped.render();

                exitpoints->deactivate();

                decorateRenderEpilog(_shader);
                _shader->deactivate();
                glPopAttrib();
                LGL_ERROR;

                data.addData(_entryImageID.getValue(), entrypoints);
                data.addData(_exitImageID.getValue(), exitpoints);
                _entryImageID.issueWrite();
                _exitImageID.issueWrite();
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image or proxy geometry found.");
        }

        _invalidationLevel.setValid();
    }

    std::string EEPGenerator::generateHeader() const {
        return getDecoratedHeader();
    }

}
