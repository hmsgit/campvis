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

#include "datacontainerinspectorcanvas.h"

#include "cgt/assert.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/datahandle.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/geometry1dtransferfunction.h"

#include "datacontainerinspectorwidget.h"

#include "ext/cgt/navigation/trackball.h"


namespace campvis {

    DataContainerInspectorCanvas::DataContainerInspectorCanvas(QWidget* parent /*= 0*/) 
        : cgt::QtThreadedCanvas("DataContainer Inspector", cgt::ivec2(640, 480), cgt::GLCanvas::RGBA_BUFFER, parent, true)
        , p_currentSlice("CurrentSlice", "Slice", -1, -1, -1)
        , p_transferFunction("TransferFunction", "Transfer Function", new Geometry1DTransferFunction(256, cgt::vec2(0.f, 1.f)))
        , p_renderRChannel("RenderRChannel", "Render Red Channel", true)
        , p_renderGChannel("RenderGChannel", "Render Green Channel", true)
        , p_renderBChannel("RenderBChannel", "Render Blue Channel", true)
        , p_renderAChannel("RenderAChannel", "Render Alpha Channel", true)
        , p_geometryRendererProperties("GeometryRendererProperties", "GeometryRenderer Properties")
        , _texturesDirty(true)
        , _dataContainer(nullptr)
        , _paintShader(nullptr)
        , _quad(nullptr)
        , _numTiles(0, 0)
        , _quadSize(0, 0)
        , _localDataContainer("Local DataContainer for DataContainerInspectorCanvas")
        , p_viewportSize("ViewportSize", "Viewport Size", cgt::ivec2(200), cgt::ivec2(0, 0), cgt::ivec2(10000))
        , _tcp(&p_viewportSize)
        , _geometryRenderer(&p_viewportSize)
    {
        static_cast<Geometry1DTransferFunction*>(p_transferFunction.getTF())->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 255), cgt::col4(255, 255, 255, 255)));

        GLCtxtMgr.registerContextAndInitGlew(this, "DataContainerInspector");
        GLCtxtMgr.releaseContext(this, false);
        
        addProperty(p_currentSlice);
        addProperty(p_transferFunction);
        addProperty(p_renderRChannel);
        addProperty(p_renderGChannel);
        addProperty(p_renderBChannel);
        addProperty(p_renderAChannel);

        p_geometryRendererProperties.addPropertyCollection(_geometryRenderer);
        _geometryRenderer.p_geometryID.setVisible(false);
        _geometryRenderer.p_textureID.setVisible(false);
        _geometryRenderer.p_renderTargetID.setVisible(false);
        _geometryRenderer.p_lightId.setVisible(false);
        _geometryRenderer.p_camera.setVisible(false);
        _geometryRenderer.p_coloringMode.setVisible(false);
        _geometryRenderer.p_pointSize.setVisible(false);
        _geometryRenderer.p_lineWidth.setVisible(false);
        _geometryRenderer.p_showWireframe.setVisible(false);
        _geometryRenderer.p_wireframeColor.setVisible(false);
        _geometryRenderer.p_renderMode.selectByOption(GL_POLYGON);
        _geometryRenderer.p_enableShading.s_changed.connect(this, &DataContainerInspectorCanvas::onGeometryRendererPropertyChanged);
        _geometryRenderer.p_renderMode.s_changed.connect(this, &DataContainerInspectorCanvas::onGeometryRendererPropertyChanged);
        _geometryRenderer.p_solidColor.s_changed.connect(this, &DataContainerInspectorCanvas::onGeometryRendererPropertyChanged);
        addProperty(p_geometryRendererProperties);

        p_geometryRendererProperties.setVisible(false);
        p_currentSlice.setVisible(false);
        p_transferFunction.setVisible(false);
        p_renderRChannel.setVisible(false);
        p_renderGChannel.setVisible(false);
        p_renderBChannel.setVisible(false);
        p_renderAChannel.setVisible(false);
    }

    DataContainerInspectorCanvas::~DataContainerInspectorCanvas() {

    }

    void DataContainerInspectorCanvas::init() {
        initAllProperties();

        _paintShader = ShdrMgr.load("core/glsl/passthrough.vert", "application/glsl/datacontainerinspector.frag", "");
        _paintShader->setAttributeLocation(0, "in_Position");
        _paintShader->setAttributeLocation(1, "in_TexCoords");
        createQuad();

        // set this as painter to get notified when window size changes
        setPainter(this, false);
        getEventHandler()->addEventListenerToFront(this);

        _geometryRenderer.init();
    }

    void DataContainerInspectorCanvas::deinit() {
        deinitAllProperties();

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        _geometryRenderer.deinit();

        _handles.clear();
        _localDataContainer.clear();
        _textures.clear();
        ShdrMgr.dispose(_paintShader);
        delete _quad;

        GLCtxtMgr.removeContext(this);
    }

    QSize DataContainerInspectorCanvas::sizeHint() const {
        return QSize(640, 480);
    }

    void DataContainerInspectorCanvas::paint() {
        tbb::mutex::scoped_lock lock(_localMutex);
        if (_texturesDirty) {
            updateTextures();
        }
        if (_geometriesDirty) {
            // update geometry renderings if necessary
            for (auto it = _geometryNames.begin(); it != _geometryNames.end(); ++it) {
                renderGeometryIntoTexture(it->first, it->second);
            }
            _geometriesDirty = false;
        }

        if (_textures.empty()) {
            return;
        }

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

        cgt::mat4 projection = cgt::mat4::createOrtho(0, size_.x, 0, size_.y, -1, 1);
        _paintShader->setUniform("_projectionMatrix", projection);

        cgt::TextureUnit tfUnit, unit2d, unit3d;
        p_transferFunction.getTF()->bind(_paintShader, tfUnit);
        _paintShader->setUniform("_texture2d", unit2d.getUnitNumber());
        _paintShader->setUniform("_texture3d", unit3d.getUnitNumber());

        _paintShader->setUniform("_renderRChannel", p_renderRChannel.getValue());
        _paintShader->setUniform("_renderGChannel", p_renderGChannel.getValue());
        _paintShader->setUniform("_renderBChannel", p_renderBChannel.getValue());
        _paintShader->setUniform("_renderAChannel", p_renderAChannel.getValue());

        for (int y = 0; y < _numTiles.y; ++y) {
            for (int x = 0; x < _numTiles.x; ++x) {
                int index = (_numTiles.x * y) + x;
                if (index >= static_cast<int>(_textures.size()))
                    break;

                // gather image
                cgtAssert(dynamic_cast<const ImageData*>(_textures[index].getData()), "Found sth. else than ImageData in render texture vector. This should not happen!");
                const ImageData* id = static_cast<const ImageData*>(_textures[index].getData());

                // compute transformation matrices
                float renderTargetRatio = static_cast<float>(_quadSize.x) / static_cast<float>(_quadSize.y);
                float sliceRatio = (static_cast<float>(id->getSize().x) * id->getMappingInformation().getVoxelSize().x)
                                 / (static_cast<float>(id->getSize().y) * id->getMappingInformation().getVoxelSize().y);
                float ratioRatio = sliceRatio / renderTargetRatio;
                cgt::mat4 viewMatrix = (ratioRatio > 1) ? cgt::mat4::createScale(cgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : cgt::mat4::createScale(cgt::vec3(ratioRatio, 1.f, 1.f));

                cgt::mat4 scaleMatrix = cgt::mat4::createScale(cgt::vec3(_quadSize, 1.f));
                cgt::mat4 translation = cgt::mat4::createTranslation(cgt::vec3(_quadSize.x * x, _quadSize.y * y, 0.f));

                _paintShader->setUniform("_modelMatrix", translation * scaleMatrix * viewMatrix);

                // render texture
                paintTexture(id->getRepresentation<ImageRepresentationGL>()->getTexture(), unit2d, unit3d);
            }
        }

        _paintShader->deactivate();
        LGL_ERROR;
        glPopAttrib();
    }

    void DataContainerInspectorCanvas::paintTexture(const cgt::Texture* texture, const cgt::TextureUnit& unit2d, const cgt::TextureUnit& unit3d) {
        cgtAssert(texture != nullptr, "Texture to paint is 0. This should not happen!");
        if (texture == nullptr)
            return;

        _paintShader->setIgnoreUniformLocationError(true);
        if (texture->getDimensions().z == 1) {
            unit2d.activate();
            texture->bind();
            _paintShader->setUniform("_is3d", false);
            _paintShader->setUniform("_isDepthTexture", texture->isDepthTexture());
            _paintShader->setUniform("_2dTextureParams._size", cgt::vec2(texture->getDimensions().xy()));
            _paintShader->setUniform("_2dTextureParams._sizeRCP", cgt::vec2(1.f) / cgt::vec2(texture->getDimensions().xy()));
            _paintShader->setUniform("_2dTextureParams._numChannels", static_cast<int>(texture->getNumChannels()));
        }
        else {
            unit3d.activate();
            texture->bind();
            _paintShader->setUniform("_is3d", true);
            _paintShader->setUniform("_sliceNumber", p_currentSlice.getValue());
            _paintShader->setUniform("_3dTextureParams._size", cgt::vec3(texture->getDimensions()));
            _paintShader->setUniform("_3dTextureParams._sizeRCP", cgt::vec3(1.f) / cgt::vec3(texture->getDimensions()));
            _paintShader->setUniform("_3dTextureParams._numChannels", static_cast<int>(texture->getNumChannels()));
        }
        _paintShader->setIgnoreUniformLocationError(false);

        _quad->render(GL_POLYGON);
        LGL_ERROR;
    }

    void DataContainerInspectorCanvas::invalidate() {
        // only if inited
        if (_quad != 0 && _paintShader != 0) {
            // avoid recursive paints.
            if (! cgt::GlContextManager::getRef().checkWhetherThisThreadHasAcquiredOpenGlContext()) {
                // TODO: check, whether this should be done in an extra thread
                cgt::GLContextScopedLock lock(this);
                paint();
            }
        }
    }

    void DataContainerInspectorCanvas::createQuad() {
        delete _quad;
        _quad = 0;
        _quad = GeometryDataFactory::createQuad(cgt::vec3(0.f), cgt::vec3(1.f), cgt::vec3(0.f, 1.f, 0.f), cgt::vec3(1.f, 0.f, 0.f));
    }

    void DataContainerInspectorCanvas::repaint() {
        invalidate();
    }

    void DataContainerInspectorCanvas::sizeChanged(const cgt::ivec2& size) {
        invalidate();
    }

    void DataContainerInspectorCanvas::mouseMoveEvent(cgt::MouseEvent* e)
    {
        if (e->modifiers() & cgt::Event::CTRL) {
            int texIndx = (e->y() / _quadSize.y) * _numTiles.x + (e->x() / _quadSize.x);
            if (texIndx < 0 || texIndx >= static_cast<int>(_textures.size()))
                return;

            const ImageData* id = static_cast<const ImageData*>(_textures[texIndx].getData());
            const cgt::Texture* tex = id->getRepresentation<ImageRepresentationGL>()->getTexture();
            const ImageRepresentationLocal* localRep = id->getRepresentation<ImageRepresentationLocal>();
            cgt::ivec2 imageSize = id->getSize().xy();

            cgt::vec2 lookupTexelFloat = cgt::vec2((e->coord() % _quadSize) * imageSize) / cgt::vec2(_quadSize);

            // compute transformation matrices
            float renderTargetRatio = static_cast<float>(_quadSize.x) / static_cast<float>(_quadSize.y);
            float sliceRatio = (static_cast<float>(id->getSize().x) * id->getMappingInformation().getVoxelSize().x)
                / (static_cast<float>(id->getSize().y) * id->getMappingInformation().getVoxelSize().y);
            float ratioRatio = sliceRatio / renderTargetRatio;

            lookupTexelFloat /= (ratioRatio > 1) ? cgt::vec2(1.f, 1.f / ratioRatio) : cgt::vec2(ratioRatio, 1.f);
            
            cgt::svec3 lookupTexel(lookupTexelFloat.x, imageSize.y - lookupTexelFloat.y, 0);
            if (lookupTexel.x >= 0 && lookupTexel.y >= 0 && lookupTexel.x < imageSize.x && lookupTexel.y < imageSize.y) {
                if (tex->isDepthTexture()) {
                    emit s_depthChanged(localRep->getElementNormalized(lookupTexel, 0));
                }
                else {
                    if (tex->getDimensions().z != 1) {
                        if (p_currentSlice.getValue() >= 0 && p_currentSlice.getValue() < tex->getDimensions().z) {
                            lookupTexel.z = static_cast<size_t>(p_currentSlice.getValue());
                            cgt::vec4 texel(0.f);
                            for (size_t i = 0; i < id->getNumChannels(); ++i) {
                                texel[i] = localRep->getElementNormalized(lookupTexel, i);
                            }
                            emit s_colorChanged(texel);
                        }
                    }
                    else if (tex->getDimensions().y != 1) {
                        cgt::vec4 texel(0.f);
                        for (size_t i = 0; i < id->getNumChannels(); ++i) {
                            texel[i] = localRep->getElementNormalized(lookupTexel, i);
                        }
                        emit s_colorChanged(texel);
                    }
                }
            }
        }
        else {
            e->ignore();
        }
    }

    void DataContainerInspectorCanvas::onEvent(cgt::Event* e) {
        cgt::EventListener::onEvent(e);
        
        if (!e->isAccepted()) {
            _tcp.onEvent(e);
            _tcp.process(_localDataContainer);
            e->accept();
            _geometriesDirty = true;
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

            _localDataContainer.clear();
            _geometryNames.clear();

            // use LightSourceProvider processor to create lighting information.
            // This is needed to be done once after the local DataContainer got cleared.
            LightSourceProvider lsp;
            lsp.init();
            lsp.invalidate(AbstractProcessor::INVALID_RESULT);
            lsp.process(_localDataContainer);
            lsp.deinit();

            // reset trackball
            resetTrackball();

            _texturesDirty = true;
        }

        invalidate();
    }

    void DataContainerInspectorCanvas::updateTextures() {
        /// Calculate the maximum slices of the textures and fill the textures array
        int maxSlices = 1;
        _textures.clear();
        p_viewportSize.setValue(cgt::ivec2(width(), height()));

        for (std::map<QString, QtDataHandle>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            if (const ImageData* img = dynamic_cast<const ImageData*>(it->second.getData())) {
                if (const ImageRepresentationGL* imgGL = img->getRepresentation<ImageRepresentationGL>()) {
                    _textures.push_back(it->second);
                    maxSlices = std::max(maxSlices, imgGL->getTexture()->getDimensions().z);
                }
            }
            else if (const RenderData* rd = dynamic_cast<const RenderData*>(it->second.getData())) {
                for (size_t i = 0; i < rd->getNumColorTextures(); ++i) {
                    const ImageRepresentationGL* imgGL = rd->getColorTexture(i)->getRepresentation<ImageRepresentationGL>();
                    if (imgGL) {
                        _textures.push_back(rd->getColorDataHandle(i));
                    }
                }
                if (rd->hasDepthTexture()) {
                    const ImageRepresentationGL* imgGL = rd->getDepthTexture()->getRepresentation<ImageRepresentationGL>();
                    if (imgGL) {
                        _textures.push_back(rd->getDepthDataHandle());
                    }

                }
            }
            else if (const GeometryData* gd = dynamic_cast<const GeometryData*>(it->second.getData())) {
                std::string name = it->first.toStdString();

                // copy geometry over to local 
                _localDataContainer.addDataHandle(name + ".geometry", it->second);

                // render
                renderGeometryIntoTexture(name);

                // store name
                _geometryNames.push_back(std::make_pair(name, static_cast<int>(_textures.size()) - 1));
            }
        }

        if (maxSlices == 1)
            maxSlices = -1;
        p_currentSlice.setMaxValue(maxSlices - 1);
        _texturesDirty = false;
        _geometriesDirty = false;
    }


    void DataContainerInspectorCanvas::onPropertyChanged(const AbstractProperty* prop) {
        // ignore properties of the geometry renderer
        if (prop != &p_geometryRendererProperties)
            invalidate();
    }

    void DataContainerInspectorCanvas::onGeometryRendererPropertyChanged(const AbstractProperty* prop) {
        _geometriesDirty = true;
        invalidate();
    }

    void DataContainerInspectorCanvas::renderGeometryIntoTexture(const std::string& name, int textureIndex) {
        // setup GeometryRenderer
        _geometryRenderer.p_geometryID.setValue(name + ".geometry");
        _geometryRenderer.p_renderTargetID.setValue(name + ".rendered");
        _geometryRenderer.validate(AbstractProcessor::INVALID_PROPERTIES);
        _geometryRenderer.invalidate(AbstractProcessor::INVALID_RESULT);
        _geometryRenderer.process(_localDataContainer);

        // grab render result texture from local DataContainer and push into texture vector.
        ScopedTypedData<RenderData> rd(_localDataContainer, name + ".rendered");
        if (rd != nullptr && rd->getNumColorTextures() > 0) {
            auto rep = rd->getColorTexture(0)->getRepresentation<ImageRepresentationGL>();
            if (rep != nullptr) {
                if (textureIndex < 0 || textureIndex >= static_cast<int>(_textures.size())) {
                    _textures.push_back(rd->getColorDataHandle(0));
                }
                else {
                    _textures[textureIndex] = rd->getColorDataHandle(0);
                }
            }
            else {
                cgtAssert(false, "The rendered geometry does not have an OpenGL representation. Something went terribly wrong.");
            }
        }
        else {
            cgtAssert(false, "The rendered geometry does exist. Something went wrong.");
        }
    }
    
    void DataContainerInspectorCanvas::resetTrackball() {
        // check whether we have to render geometries
        cgt::Bounds unionBounds;
        for (std::map<QString, QtDataHandle>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            if (const GeometryData* gd = dynamic_cast<const GeometryData*>(it->second.getData())) {
                unionBounds.addVolume(gd->getWorldBounds());
            }
        }

        // if so, create a new trackball
        if (unionBounds.isDefined()) {
            _tcp.reinitializeCamera(unionBounds);
        }

        _tcp.invalidate(AbstractProcessor::INVALID_RESULT);
        _tcp.process(_localDataContainer);
    }

}
