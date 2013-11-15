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
#include "core/tools/job.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/geometry1dtransferfunction.h"

#include "datacontainerinspectorwidget.h"

#include "ext/tgt/navigation/trackball.h"


namespace campvis {

    DataContainerInspectorCanvas::DataContainerInspectorCanvas(QWidget* parent /*= 0*/) 
        : tgt::QtThreadedCanvas("DataContainer Inspector", tgt::ivec2(640, 480), tgt::GLCanvas::RGBA_BUFFER, parent, true)
        , p_currentSlice("CurrentSlice", "Slice", -1, -1, -1)
        , p_meshSolidColor("MeshSolidColor", "Mesh Solid Color", tgt::vec4(50.f, 70.0f, 50.0f, 255.f), tgt::vec4(0.0f), tgt::vec4(255.0f))
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
		, _meshGeomTexturesDirty(false)
		, _renderingWndSize(tgt::ivec2(400, 100))
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
        addProperty(&p_meshSolidColor);
        //addProperty(&_camera);
    }

    DataContainerInspectorCanvas::~DataContainerInspectorCanvas() {

    }

    void DataContainerInspectorCanvas::init(DataContainerInspectorWidget* _pWidget) {
        initAllProperties();

        _widget = _pWidget;

        GLJobProc.registerContext(this);
        _paintShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "application/glsl/datacontainerinspector.frag", "", false);
		_geomteryRenderingShader = ShdrMgr.loadSeparate("core/glsl/meshgeomteryrenderer.vert", "modules/vis/glsl/meshgeometryrenderer.frag", "", false);

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
        LGL_ERROR;
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

	void DataContainerInspectorCanvas::paintMeshGeomTextures() {
        LGL_ERROR;
        tbb::mutex::scoped_lock lock(_localMutex);
        if (_meshGeomTexturesDirty)
			updateMeshGeomRenderedTextures();

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
        LGL_ERROR;
    }

	void DataContainerInspectorCanvas::drawMeshGeomtery(const campvis::MeshGeometry* mg, tgt::Texture* colorBuffer, tgt::Texture* depthBuffer, const unsigned int& meshIndex)
    {
        LGL_ERROR;
        // Here the object will be rendered into a texture and the texture will be shown on the output buffer
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		_geomteryRenderingShader->activate();
		LGL_ERROR;
		_geomteryRenderingShader->setIgnoreUniformLocationError(true);
		LGL_ERROR;
		//decorateRenderProlog(data, _shader);
				
        //_trackballCamera->setPosition(tgt::vec3(0.0f, 0.0f, dist));
        //_trackballCamera->setFocus(tgt::vec3(0, 0, -1));
        //_trackballCamera->setFarDist(1000.0f);

        _trackballEHs[meshIndex]->setSceneBounds(mg->getWorldBounds());
				
		campvis::IVec4Property* p_color = new campvis::IVec4Property("myColor", "MyRenderingColor", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f));
		
        _geomteryRenderingShader->setUniform("_projectionMatrix", _trackballEHs[meshIndex]->_trackball->getCamera()->getProjectionMatrix()/*_trackballCameraProperty->getValue().getProjectionMatrix()*/);
        LGL_ERROR;
        _geomteryRenderingShader->setUniform("_viewMatrix", _trackballEHs[meshIndex]->_trackball->getCamera()->getViewMatrix());
        LGL_ERROR;

        tgt::Vector4<float> meshColor = static_cast<tgt::Vector4<float>>(p_meshSolidColor.getValue());
        meshColor.r /= 255.0f;
        meshColor.g /= 255.0f;
        meshColor.b /= 255.0f;
        meshColor.a /= 255.0f;
        _geomteryRenderingShader->setUniform("_Color", meshColor);
        
        LGL_ERROR;
        _geomteryRenderingShader->setUniform("_cameraPosition", _trackballEHs[meshIndex]->_trackball->getCamera()->getPosition());
		_geomteryRenderingShader->setIgnoreUniformLocationError(false);
						LGL_ERROR;
		tgt::FramebufferObject* frameBuffer = new tgt::FramebufferObject();
						LGL_ERROR;
		frameBuffer->activate();
		LGL_ERROR;

		// acqiure a new TextureUnit, so that we don't mess with other currently bound textures during texture upload...
		//tgt::TextureUnit rtUnit;
		//rtUnit.activate();

		// Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        colorBuffer->uploadTexture();
		colorBuffer->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

		depthBuffer->uploadTexture();
		depthBuffer->setWrapping(tgt::Texture::CLAMP_TO_EDGE);
		LGL_ERROR;
		frameBuffer->attachTexture(colorBuffer, GL_COLOR_ATTACHMENT0);
		frameBuffer->attachTexture(depthBuffer, GL_DEPTH_ATTACHMENT);
		frameBuffer->isComplete();
		LGL_ERROR;

		glViewport(0, 0, _renderingWndSize.x, _renderingWndSize.y);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		LGL_ERROR;

        //for (std::map<QString, QtDataHandle>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
        //    if (const campvis::MeshGeometry* mg = dynamic_cast<const campvis::MeshGeometry*>(it->second.getData())) {
        
		glPolygonMode(GL_FRONT, GL_POLYGON);
        mg->render(GL_POLYGON);


        _geomteryRenderingShader->setUniform("_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        // Render wireframe around the object
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mg->render(GL_POLYGON);
       //     }
      //  }
		
		
        
        LGL_ERROR;

		colorBuffer->downloadTexture();
		depthBuffer->downloadTexture();

		frameBuffer->deactivate();
		LGL_ERROR;

		_geomteryRenderingShader->deactivate();

        glPopAttrib();

        //delete frameBuffer;
    }

    void DataContainerInspectorCanvas::invalidate() {
        // only if inited
        if (_quad != 0 && _paintShader != 0)
            GLJobProc.enqueueJob(this, makeJobOnHeap(this, &DataContainerInspectorCanvas::paint), OpenGLJobProcessor::PaintJob);
    }

	void DataContainerInspectorCanvas::invalidateMeshGeomTextures() {
        // only if inited
        if (_quad != 0 && _paintShader != 0)
			GLJobProc.enqueueJob(this, makeJobOnHeap(this, &DataContainerInspectorCanvas::paintMeshGeomTextures), OpenGLJobProcessor::PaintJob);
		//paintMeshGeomTextures();
    }

    void DataContainerInspectorCanvas::createQuad() {
        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3(0.f,  0.f, 0.f));
        vertices.push_back(tgt::vec3(1.f, 0.f, 0.f));
        vertices.push_back(tgt::vec3(1.f, 1.f, 0.f));
        vertices.push_back(tgt::vec3(0.f,  1.f, 0.f));
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
        //LGL_ERROR;
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
        if(e->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT)
        {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);

            tgt::MouseEvent adjustedMe(
                me->x(),
                me->y(),
                me->action(),
                me->modifiers(),
                me->button(),
                me->viewport() 
                );

			
			if(_selectedTrackball >= 0)
			{
				_trackballEHs[_selectedTrackball]->onEvent(&adjustedMe);

				_meshGeomTexturesDirty = true;
				invalidateMeshGeomTextures();
			}
            
        }
        else if(e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT)
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

			float f = 0.0;
			if(_textures[texIndx]->isDepthTexture())
			{
				_depth = _textures[texIndx]->depthAsFloat(cursorPosX, texHeight - cursorPosY - 1);
				_widget->updateDepth();
			}
			else
			{
				_color = _textures[texIndx]->texelAsFloat(cursorPosX, texHeight - cursorPosY - 1);
				_widget->updateColor();
			}      
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

    void DataContainerInspectorCanvas::mousePressEvent(tgt::MouseEvent* e) {
        //LGL_ERROR;
        //if (_renderFullscreen) {

			tgt::ivec2 selectedIndex(e->x() / _quadSize.x, e->y() / _quadSize.y);
			_selectedTrackball = -1;
			int geomTextureIndex = 0;

            switch (e->button()) {
            case tgt::MouseEvent::MOUSE_BUTTON_LEFT:
                    //++_currentSlice; // we cant clamp the value here to the number of slices - we do this during rendering

				_selectedTexture = (selectedIndex.y * _numTiles.x) + selectedIndex.x;

				while(geomTextureIndex < _geomTextureIndices.size() && _geomTextureIndices[geomTextureIndex] <= _selectedTexture)
				{
					if(_geomTextureIndices[geomTextureIndex] == _selectedTexture)
					{
						_selectedTrackball = geomTextureIndex;
						break;
					}

					geomTextureIndex++;
				}

				if(_selectedTrackball >= 0)
					_trackballEHs[_selectedTrackball]->_trackball->startMouseDrag(e);
                   // e->ignore();
                    break;
                
                default:
                    break;
            }
        //}
    }

    void DataContainerInspectorCanvas::mouseReleaseEvent(tgt::MouseEvent* e) {
        if (_renderFullscreen) {
            switch (e->button()) {
            case tgt::MouseEvent::MOUSE_BUTTON_LEFT:
                    //++_currentSlice; // we cant clamp the value here to the number of slices - we do this during rendering
				if(_selectedTrackball >= 0)
                _trackballEHs[_selectedTrackball]->_trackball->endMouseDrag(e);
                   // e->ignore();
                    break;
                
                default:
                    break;
            }
            //invalidate();
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

	const float& DataContainerInspectorCanvas::getCapturedDepth()
    {
        return _depth;
    }

    void DataContainerInspectorCanvas::updateTextures() {
        /// Clear the textures Array
        _textures.clear();
		_geomTextureIndices.clear();

		/// Clear the geomtery rendered textures Array
		std::vector<TrackballNavigationEventListener*>::iterator trackballNavEHIterator = _trackballEHs.begin();
		for(; trackballNavEHIterator != _trackballEHs.end(); trackballNavEHIterator++)
		{
			delete (*trackballNavEHIterator);
		}
		_trackballEHs.clear();

        std::vector<campvis::CameraProperty*>::iterator camPropertyIterator = _trackballCameraProperties.begin();
		for(; camPropertyIterator != _trackballCameraProperties.end(); camPropertyIterator++)
		{
			delete (*camPropertyIterator);
		}
        _trackballCameraProperties.clear();

        /// Calculate the maximum slices of the textures and fill the textures array
        int maxSlices = 1;
		unsigned int nMeshGeometry = 0;
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
				
                LGL_ERROR;
                
                static bool flag = false;

				campvis::CameraProperty* cameraProperty = new CameraProperty("camera", "Camera");
				_trackballCameraProperties.push_back(cameraProperty);

                TrackballNavigationEventListener* trackballEH = new TrackballNavigationEventListener(cameraProperty, new IVec2Property("QuadSize", "Size", _renderingWndSize, tgt::ivec2(0), tgt::ivec2(1024)) );
                //if(!flag)
                //{
                    float dist = 3 * fabs(mg->getWorldBounds().getLLF().z - mg->getWorldBounds().getURB().z);
                    trackballEH->reinitializeCamera(mg->getWorldBounds());
                    trackballEH->_trackball->moveCameraBackward(dist);
                    LGL_ERROR;
                    flag = true;
                //}
				_trackballEHs.push_back(trackballEH);
				
				tgt::Texture* colorBuffer = new tgt::Texture(0, tgt::ivec3(_renderingWndSize.x, _renderingWndSize.y, 1), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
				tgt::Texture* depthBuffer  = new tgt::Texture(0, tgt::ivec3(_renderingWndSize.x, _renderingWndSize.y, 1), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);

                glewExperimental = true;
                drawMeshGeomtery(mg, colorBuffer, depthBuffer, nMeshGeometry++);
                LGL_ERROR;
                
                _geomteryRendering_ColorBuffers.push_back(colorBuffer);
                _geomteryRendering_DepthBuffers.push_back(depthBuffer);
				_textures.push_back(colorBuffer);
				_geomTextureIndices.push_back(_textures.size() - 1);

            }else if(const campvis::FaceGeometry* fg = dynamic_cast<const campvis::FaceGeometry*>(it->second.getData())){


			}
        }

        if (maxSlices == 1)
            maxSlices = -1;
        p_currentSlice.setMaxValue(maxSlices);
        _texturesDirty = false;
    }

	void DataContainerInspectorCanvas::updateMeshGeomRenderedTextures() {
        
		/// Calculate the maximum slices of the textures and fill the textures array
        int maxSlices = 1;
		unsigned int nMeshGeometry = 0;
		unsigned int nElement = 0;
        for (std::map<QString, QtDataHandle>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            if (const ImageData* img = dynamic_cast<const ImageData*>(it->second.getData())) {
                if (const ImageRepresentationGL* imgGL = img->getRepresentation<ImageRepresentationGL>()) {
                    nElement++;
                }
            }
            else if (const RenderData* rd = dynamic_cast<const RenderData*>(it->second.getData())) {
                for (size_t i = 0; i < rd->getNumColorTextures(); ++i) {
                    const ImageRepresentationGL* imgGL = rd->getColorTexture(i)->getRepresentation<ImageRepresentationGL>();
                    if (imgGL)
                    {
						nElement++;
                    }
                }
                if (rd->hasDepthTexture()) {
                    const ImageRepresentationGL* imgGL = rd->getDepthTexture()->getRepresentation<ImageRepresentationGL>();
                    if (imgGL)
                    {
                        nElement++;
                    }
                }
            }
            else if(const campvis::MeshGeometry* mg = dynamic_cast<const campvis::MeshGeometry*>(it->second.getData())){
				
                LGL_ERROR;
                
				tgt::Texture* colorBuffer = new tgt::Texture(0, tgt::ivec3(_renderingWndSize.x, _renderingWndSize.y, 1), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
				tgt::Texture* depthBuffer  = new tgt::Texture(0, tgt::ivec3(_renderingWndSize.x, _renderingWndSize.y, 1), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);

                //_meshGeomteryPtr = mg;
				delete *(_textures.begin() + nElement);
				std::vector<const tgt::Texture*>::iterator erased = _textures.erase(_textures.begin() + nElement);

				_trackballCameraProperties[nMeshGeometry]->getValue();
				
                //drawMeshGeomtery(mg, _geomteryRendering_ColorBuffers[nMeshGeometry], _geomteryRendering_DepthBuffers[nMeshGeometry], nMeshGeometry);
                glewExperimental = true;
                drawMeshGeomtery(mg, colorBuffer, depthBuffer, nMeshGeometry);
                nMeshGeometry++;
				_textures.insert(_textures.begin() + nElement, colorBuffer);

				nElement++;

            }else if(const campvis::FaceGeometry* fg = dynamic_cast<const campvis::FaceGeometry*>(it->second.getData())){

				
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

        /// if the Mesh Solid Color property is changed, update the mesh's color
        const std::string propertyName = (prop)->getName();
        if(propertyName == "MeshSolidColor")
        {
            _meshGeomTexturesDirty = true;
            invalidateMeshGeomTextures();
        }        
    }

}
