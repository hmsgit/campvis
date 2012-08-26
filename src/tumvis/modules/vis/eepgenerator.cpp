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
#include "tgt/quadrenderer.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace TUMVis {
    const std::string EEPGenerator::loggerCat_ = "TUMVis.modules.vis.EEPGenerator";

    EEPGenerator::EEPGenerator(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _entryImageID("entryImageID", "Output Entry Points Image", "")
        , _exitImageID("exitImageID", "Output Exit Points Image", "")
        , _camera("camera", "Camera")
        , _shader(0)
    {
        addProperty(&_sourceImageID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_camera);
    }

    EEPGenerator::~EEPGenerator() {

    }

    void EEPGenerator::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/eepgenerator.frag", "", false);
    }

    void EEPGenerator::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void EEPGenerator::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                // TODO: implement some kind of cool proxy geometry supporting clipping (camera in volume)...
                tgt::Bounds volumeExtent = img->getWorldBounds();
                tgt::Bounds textureBounds(tgt::vec3(0.f), tgt::vec3(1.f));

                // set modelview and projection matrices
                glPushAttrib(GL_ALL_ATTRIB_BITS);

                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                tgt::loadMatrix(_camera.getValue().getProjectionMatrix());

                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                tgt::loadMatrix(_camera.getValue().getViewMatrix());

                _shader->activate();
                glEnable(GL_CULL_FACE);

                // create entry points texture
                ImageDataRenderTarget* entrypoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1), GL_RGBA16);
                entrypoints->activate();

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(GL_BACK);
                tgt::QuadRenderer::renderCube(volumeExtent, textureBounds);

                entrypoints->deactivate();

                // create exit points texture
                ImageDataRenderTarget* exitpoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1), GL_RGBA16);
                exitpoints->activate();

                glDepthFunc(GL_GREATER);
                glClearDepth(0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(GL_FRONT);
                tgt::QuadRenderer::renderCube(volumeExtent, textureBounds);

                exitpoints->deactivate();

                _shader->deactivate();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glPopAttrib();

                data.addData(_entryImageID.getValue(), entrypoints);
                data.addData(_exitImageID.getValue(), exitpoints);
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

}
