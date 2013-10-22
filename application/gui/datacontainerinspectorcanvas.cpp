// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "datacontainerinspectorcanvas.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/datahandle.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/meshgeometry.h"
#include "core/tools/job.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/geometry1dtransferfunction.h"

#include "datacontainerinspectorwidget.h"


namespace campvis {

    DataContainerInspectorCanvas::DataContainerInspectorCanvas(QWidget* parent /*= 0*/) 
        : tgt::QtThreadedCanvas("DataContainer Inspector", tgt::ivec2(640, 480), tgt::GLCanvas::RGBA_BUFFER, parent, true)
        , p_currentSlice("CurrentSlice", "Slice", -1, -1, -1)
        , p_transferFunction("TransferFunction", "Transfer Function", new Geometry1DTransferFunction(256, tgt::vec2(0.f, 1.f)))
        , _dataContainer(0)
        , _paintShader(0)
        , _quad(0)
        , _numTiles(0, 0)
        , _quadSize(0, 0)
        , _selectedTexture(0)
        , _renderFullscreen(false)
        , _currentSlice(-1)
        , _color(0.0f, 0.0f, 0.0f, 0.0f)
    {
        static_cast<Geometry1DTransferFunction*>(p_transferFunction.getTF())->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));

        makeCurrent();
        // Init GLEW for this context
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            tgtAssert(false, "glewInit failed");
            std::cerr << "glewInit failed, error: " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
        }

        addProperty(&p_currentSlice);
        addProperty(&p_transferFunction);
    }

    DataContainerInspectorCanvas::~DataContainerInspectorCanvas() {

    }

    void DataContainerInspectorCanvas::init(DataContainerInspectorWidget* _pWidget) {
        initAllProperties();

        _widget = _pWidget;

        GLJobProc.registerContext(this);
        _paintShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "application/glsl/datacontainerinspector.frag", "", false);
        _paintShader->setAttributeLocation(0, "in_Position");
        _paintShader->setAttributeLocation(1, "in_TexCoords");

        createQuad();

        // set this as painter to get notified when window size changes
        setPainter(this, false);
        getEventHandler()->addEventListenerToFront(this);
    }

    void DataContainerInspectorCanvas::deinit() {
        deinitAllProperties();

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        _handles.clear();
        GLJobProc.deregisterContext(this);
        ShdrMgr.dispose(_paintShader);
        delete _quad;
    }

    QSize DataContainerInspectorCanvas::sizeHint() const {
        return QSize(640, 480);
    }


    void DataContainerInspectorCanvas::paint() {
        tbb::mutex::scoped_lock lock(_localMutex);
        if (_texturesDirty)
            updateTextures();

        if (_textures.empty())
            return;

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glViewport(0, 0, size_.x, size_.y);
        glClearColor(0.7f, 0.7f, 0.7f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        LGL_ERROR;

        // update layout dimensions
        _numTiles.x = ceil(sqrt(static_cast<float>(_textures.size())));
        _numTiles.y = ceil(static_cast<float>(_textures.size()) / _numTiles.x);
        _quadSize = size_ / _numTiles;

        _paintShader->activate();

        tgt::mat4 projection = tgt::mat4::createOrtho(0, size_.x, 0, size_.y, -1, 1);
        _paintShader->setUniform("_projectionMatrix", projection);

        tgt::TextureUnit tfUnit, unit2d, unit3d;
        p_transferFunction.getTF()->bind(_paintShader, tfUnit);
        _paintShader->setUniform("_texture2d", unit2d.getUnitNumber());
        _paintShader->setUniform("_texture3d", unit3d.getUnitNumber());

        for (int y = 0; y < _numTiles.y; ++y) {
            for (int x = 0; x < _numTiles.x; ++x) {
                int index = (_numTiles.x * y) + x;
                if (index >= static_cast<int>(_textures.size()))
                    break;

                tgt::mat4 scaleMatrix = tgt::mat4::createScale(tgt::vec3(_quadSize, 1.f));
                tgt::mat4 translation = tgt::mat4::createTranslation(tgt::vec3(_quadSize.x * x, _quadSize.y * y, 0.f));
                _paintShader->setUniform("_modelMatrix", translation * scaleMatrix);
                paintTexture(_textures[index], unit2d, unit3d);
            }
        }

        _paintShader->deactivate();
        LGL_ERROR;
        glPopAttrib();
    }

    void DataContainerInspectorCanvas::paintTexture(const tgt::Texture* texture, const tgt::TextureUnit& unit2d, const tgt::TextureUnit& unit3d) {

        _paintShader->setIgnoreUniformLocationError(true);
        if (texture->getDimensions().z == 1) {
            unit2d.activate();
            texture->bind();
            _paintShader->setUniform("_is3d", false);
            _paintShader->setUniform("_isDepthTexture", texture->isDepthTexture());
            _paintShader->setUniform("_2dTextureParams._size", tgt::vec2(texture->getDimensions().xy()));
            _paintShader->setUniform("_2dTextureParams._sizeRCP", tgt::vec2(1.f) / tgt::vec2(texture->getDimensions().xy()));
            _paintShader->setUniform("_2dTextureParams._numChannels", static_cast<int>(texture->getNumChannels()));
        }
        else {
            // clamp current slice to texture size, since this can't be done in event handler:
            _currentSlice = tgt::clamp(_currentSlice, -1, texture->getDimensions().z);

            unit3d.activate();
            texture->bind();
            _paintShader->setUniform("_is3d", true);
            _paintShader->setUniform("_sliceNumber", p_currentSlice.getValue());
            _paintShader->setUniform("_3dTextureParams._size", tgt::vec3(texture->getDimensions()));
            _paintShader->setUniform("_3dTextureParams._sizeRCP", tgt::vec3(1.f) / tgt::vec3(texture->getDimensions()));
            _paintShader->setUniform("_3dTextureParams._numChannels", static_cast<int>(texture->getNumChannels()));
        }
        _paintShader->setIgnoreUniformLocationError(false);

        _quad->render(GL_POLYGON);
    }

    void DataContainerInspectorCanvas::invalidate() {
        // only if inited
        if (_quad != 0 && _paintShader != 0)
            GLJobProc.enqueueJob(this, makeJobOnHeap(this, &DataContainerInspectorCanvas::paint), OpenGLJobProcessor::PaintJob);
    }

    void DataContainerInspectorCanvas::createQuad() {
        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3( 0.f,  0.f, 0.f));
        vertices.push_back(tgt::vec3(1.f, 0.f, 0.f));
        vertices.push_back(tgt::vec3(1.f, 1.f, 0.f));
        vertices.push_back(tgt::vec3( 0.f,  1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 1.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 1.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 0.f, 0.f));

        delete _quad;
        _quad = new FaceGeometry(vertices, texCorods);
        _quad->createGLBuffers();
    }

    void DataContainerInspectorCanvas::repaint() {
        invalidate();
    }

    void DataContainerInspectorCanvas::sizeChanged(const tgt::ivec2&) {
        invalidate();
    }

    void DataContainerInspectorCanvas::mouseDoubleClickEvent(tgt::MouseEvent* e) {
        if (_renderFullscreen) {
            _renderFullscreen = false;
        }
        else {
            tgt::ivec2 selectedIndex(e->x() / _quadSize.x, e->y() / _quadSize.y);
            _selectedTexture = (selectedIndex.y * _numTiles.x) + selectedIndex.x;
            _renderFullscreen = true;
        }
        e->ignore();
        invalidate();
    }

    void DataContainerInspectorCanvas::mouseMoveEvent(tgt::MouseEvent* e)
    {
        /*if (_renderFullscreen) {
            _renderFullscreen = false;
        }
        else {
            tgt::ivec2 selectedIndex(e->x() / _quadSize.x, e->y() / _quadSize.y);
            _selectedTexture = (selectedIndex.y * _numTiles.x) + selectedIndex.x;
            _renderFullscreen = true;
        }
        e->ignore();
        invalidate();*/
        
        if(e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT)
        {

            tgt::ivec2 dimCanvas = tgt::ivec2(_quadSize.x * _numTiles.x, _quadSize.y * _numTiles.y);    
            if(e->x() >= dimCanvas.x || e->y() >= dimCanvas.y || e->x() < 0 || e->y() < 0)
                return;

            int texIndx = (e->y() / _quadSize.y) * _numTiles.x + (e->x() / _quadSize.x);
            const tgt::Texture* texturePtr = _textures[texIndx];
            const int texWidth  = texturePtr->getWidth();
            const int texHeight = texturePtr->getHeight();
            int cursorPosX = static_cast<int>(static_cast<float>(e->x() % _quadSize.x) / _quadSize.x * texWidth);
            int cursorPosY = static_cast<int>(static_cast<float>(e->y() % _quadSize.y) / _quadSize.y * texHeight);
            _color = _textures[texIndx]->texelAsFloat(cursorPosX, texHeight - cursorPosY - 1);
            
            _widget->updateColor();
        }
    }

    void DataContainerInspectorCanvas::wheelEvent(tgt::MouseEvent* e) {
        if (_renderFullscreen) {
            switch (e->button()) {
                case tgt::MouseEvent::MOUSE_WHEEL_UP:
                    ++_currentSlice; // we cant clamp the value here to the number of slices - we do this during rendering
                    e->ignore();
                    break;
                case tgt::MouseEvent::MOUSE_WHEEL_DOWN:
                    if (_currentSlice >= -1)
                        --_currentSlice;
                    e->ignore();
                    break;
                default:
                    break;
            }
            invalidate();
        }
    }

    void DataContainerInspectorCanvas::onDataContainerChanged(const QString& key, QtDataHandle dh) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);

            // check whether DataHandle is already existing
            std::map<QString, QtDataHandle>::iterator lb = _handles.lower_bound(key);
            if (lb == _handles.end() || lb->first != key) {
                // not existant -> do nothing
            }
            else {
                // existant -> replace
                lb->second = QtDataHandle(dh);
                // update _textures array
                _texturesDirty = true;
            }
        }

        if (_texturesDirty)
            invalidate();
    }

    void DataContainerInspectorCanvas::setDataHandles(const std::vector< std::pair<QString, QtDataHandle> >& handles) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _handles.clear();
            for (std::vector< std::pair<QString, QtDataHandle> >::const_iterator it = handles.begin(); it != handles.end(); ++it)
                _handles.insert(*it);

            _texturesDirty = true;
        }

        invalidate();
    }

    const tgt::Color& DataContainerInspectorCanvas::getCapturedColor()
    {
        return _color;
    }

    void DataContainerInspectorCanvas::updateTextures() {
        /// Clear the textures Array
        _textures.clear();

        /// Calculate the maximum slices of the textures and fill the textures array
        int maxSlices = 1;
        for (std::map<QString, QtDataHandle>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            if (const ImageData* img = dynamic_cast<const ImageData*>(it->second.getData())) {
                if (const ImageRepresentationGL* imgGL = img->getRepresentation<ImageRepresentationGL>()) {
                    _textures.push_back(imgGL->getTexture());
                    maxSlices = std::max(maxSlices, imgGL->getTexture()->getDimensions().z);
                }
            }
            else if (const RenderData* rd = dynamic_cast<const RenderData*>(it->second.getData())) {
                for (size_t i = 0; i < rd->getNumColorTextures(); ++i) {
                    const ImageRepresentationGL* imgGL = rd->getColorTexture(i)->getRepresentation<ImageRepresentationGL>();
                    if (imgGL)
                    {
                        imgGL->downloadTexture();
                        _textures.push_back(imgGL->getTexture());
                    }
                }
                if (rd->hasDepthTexture()) {
                    const ImageRepresentationGL* imgGL = rd->getDepthTexture()->getRepresentation<ImageRepresentationGL>();
                    if (imgGL)
                    {
                        imgGL->downloadTexture();
                        _textures.push_back(imgGL->getTexture());
                    }

                }
            }
            else if(const campvis::MeshGeometry* mg = dynamic_cast<const campvis::MeshGeometry*>(it->second.getData())){

                // Here the object will be rendered into a texture and the texture will be shown on the output buffer.

                //mg->render(GL_POLYGON);
                //const tgt::BufferObject* buffer = mg->getColorsBuffer();
            }
        }

        if (maxSlices == 1)
            maxSlices = -1;
        p_currentSlice.setMaxValue(maxSlices);
        _texturesDirty = false;
    }

    void DataContainerInspectorCanvas::onPropertyChanged(const AbstractProperty* prop) {
        HasPropertyCollection::onPropertyChanged(prop);
        invalidate();
    }

}
