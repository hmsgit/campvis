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

        // TODO: remove this ugly hack: automatically adapt near/far plane to volume extent.
        tgt::Camera c;
        c.setFarDist(512.f);
        _camera.setValue(c);
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
                ImageDataRenderTarget* entrypoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
                entrypoints->activate();

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(GL_BACK);
                tgt::QuadRenderer::renderCube(volumeExtent, textureBounds);

                entrypoints->deactivate();

                // create exit points texture
                ImageDataRenderTarget* exitpoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
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