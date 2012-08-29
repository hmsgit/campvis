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

#include "datacontainerinspectorcanvas.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/datahandle.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedatagl.h"
#include "core/tools/job.h"
#include "application/gui/datacontainertreewidget.h"

namespace TUMVis {

    DataContainerInspectorCanvas::DataContainerInspectorCanvas(QWidget* parent /*= 0*/) 
        : tgt::QtThreadedCanvas("DataContainer Inspector", tgt::ivec2(640, 480), tgt::GLCanvas::RGBA_BUFFER, parent, true)
        , _dataContainer(0)
        , _paintShader(0)
    {

    }

    DataContainerInspectorCanvas::~DataContainerInspectorCanvas() {

    }

    void DataContainerInspectorCanvas::init() {

    }

    void DataContainerInspectorCanvas::deinit() {

    }

    void DataContainerInspectorCanvas::setDataContainer(DataContainer* dataContainer) {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _dataContainer = dataContainer;
            for (std::map<std::string, const DataHandle*>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
                delete it->second;
            }
            _handles = _dataContainer->getHandlesCopy();
        }

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.connect(this, &DataContainerInspectorCanvas::onDataContainerDataAdded);
        }

        invalidate();
    }

    QSize DataContainerInspectorCanvas::sizeHint() const {
        return QSize(640, 480);
    }

    void DataContainerInspectorCanvas::onDataContainerDataAdded(const std::string& name, const DataHandle* dh) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);

            // check whether DataHandle is already existing
            std::map<std::string, const DataHandle*>::iterator lb = _handles.lower_bound(name);
            if (lb == _handles.end() || lb->first != name) {
                // not existant
                _handles.insert(std::make_pair(name, new DataHandle(*dh)));
            }
            else {
                delete lb->second;
                lb->second = new DataHandle(*dh);
            }
        }

        invalidate();
    }

    void DataContainerInspectorCanvas::paint() {
        tbb::mutex::scoped_lock lock(_localMutex);

        std::vector<const tgt::Texture*> textures;
        for (std::map<std::string, const DataHandle*>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            if (const ImageDataGL* imgGL = dynamic_cast<const ImageDataGL*>(it->second)) {
            	textures.push_back(imgGL->getTexture());
            }
            else if (const ImageDataRenderTarget* imgRT = dynamic_cast<const ImageDataRenderTarget*>(it->second)) {
            	textures.push_back(imgRT->getColorTexture());
                textures.push_back(imgRT->getDepthTexture());
            }
        }

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glEnable(GL_TEXTURE_2D);
        glClearColor(0.7f, 0.7f, 0.7f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(1.f, 1.f, 1.f, 1.f);

        LGL_ERROR;

        if (_handles.empty()) {
            glPopAttrib();
            return;
        }

        // update window title
        int memsize = 0;
        for (size_t i = 0; i < textures.size(); ++i) {
            memsize += textures[i]->getSizeOnGPU();
        }
        memsize /= 1024 * 1024;
        QString title = tr("DataContainer Inspector: %1 Textures (%2 mb)").arg(textures.size()).arg(memsize);
        if (parentWidget() && parentWidget()->parentWidget())
            parentWidget()->parentWidget()->setWindowTitle(title);
        else
            setWindowTitle(title);

        // render port contents
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(0, size_.x, 0, size_.y, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        LGL_ERROR;

//         if (textures.empty()) {
//             renderFont(13, 13, "No rendertargets selected.");
//             glPopMatrix();
//             glMatrixMode(GL_PROJECTION);
//             glPopMatrix();
//             glPopAttrib();
//             return;
//         }

        // update layout dimensions
        dimX_ = (int)ceil(sqrt((float)textures.size()));
        dimY_ = ceil((float)textures.size() / dimX_);

        scaledWidth_ = size_.x / dimX_;
        scaledHeight_ = size_.y / dimY_;

        if (fullscreen_) {
            if(selected_ >= 0 && selected_ < (int)textures.size()) {
                paintTexture(textures[selected_]);
            }
        }
        else {
            for (int y = 0; y < dimY_; ++y) {
                for (int x = 0; x < dimX_; ++x) {
                    int index = (dimX_ * y) + x;
                    if (index >= static_cast<int>(textures.size()))
                        break;

                    glPushMatrix();
                    glTranslatef(scaledWidth_ * x, scaledHeight_ * y, 0.0);
                    glScalef(1.f, 1.f, 1.f);
                    paintTexture(textures[index]);
                    glPopMatrix();
                }
            }
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        LGL_ERROR;

        glPopAttrib();
    }

    void DataContainerInspectorCanvas::paintTexture(const tgt::Texture* texture) {
        _paintShader->activate();
    }

    void DataContainerInspectorCanvas::invalidate() {
        GLJobProc.enqueueJob(this, new CallMemberFuncJob<DataContainerInspectorCanvas>(this, &DataContainerInspectorCanvas::paint), OpenGLJobProcessor::PaintJob);
    }

}