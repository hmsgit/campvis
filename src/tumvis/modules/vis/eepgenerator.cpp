#include "eepgenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/quadrenderer.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

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
        VisualizationProcessor::deinit();
    }

    void EEPGenerator::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                // TODO: implement some kind of proxy geometry...
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

                glDepthFunc(GL_GREATER);
                glClearDepth(0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(GL_FRONT);
                renderProxyGeometry(volumeExtent, textureBounds);

                entrypoints->deactivate();

                // create exit points texture
                ImageDataRenderTarget* exitpoints = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
                exitpoints->activate();

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(GL_BACK);
                renderProxyGeometry(volumeExtent, textureBounds);

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

    void EEPGenerator::renderProxyGeometry(const tgt::Bounds& bounds, const tgt::Bounds& texBounds) {
        const tgt::vec3& llf = bounds.getLLF();
        const tgt::vec3& urb = bounds.getURB();
        const tgt::vec3& tLlf = texBounds.getLLF();
        const tgt::vec3& tUrb = texBounds.getURB();

        // TODO: get fuckin' rid of intermediate mode, it simply sucks...
        glColor3f(1.f, 0.f, 1.f);
        glBegin(GL_QUADS);
            // front
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);

            // right
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);

            // top
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);
            glTexCoord3f(tUrb.x, tUrb.y, tLlf.z);
            glVertex3f(urb.x, urb.y, llf.z);
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);

            // left
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);
            glTexCoord3f(tLlf.x, tUrb.y, tLlf.z);
            glVertex3f(llf.x, urb.y, llf.z);
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);

            // bottom
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
            glTexCoord3f(tUrb.x, tLlf.y, tLlf.z);
            glVertex3f(urb.x, llf.y, llf.z);
            glTexCoord3f(tLlf.x, tLlf.y, tLlf.z);
            glVertex3f(llf.x, llf.y, llf.z);

            // back
            glTexCoord3f(tUrb.x, tLlf.y, tUrb.z);
            glVertex3f(urb.x, llf.y, urb.z);
            glTexCoord3f(tLlf.x, tLlf.y, tUrb.z);
            glVertex3f(llf.x, llf.y, urb.z);
            glTexCoord3f(tLlf.x, tUrb.y, tUrb.z);
            glVertex3f(llf.x, urb.y, urb.z);
            glTexCoord3f(tUrb.x, tUrb.y, tUrb.z);
            glVertex3f(urb.x, urb.y, urb.z);
        glEnd();
    }

}
