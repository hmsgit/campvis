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

#include "microscopyimagesegmentation.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/event/keyevent.h"
#include "cgt/event/mouseevent.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/tools/quadrenderer.h"

#include "modules/microscopysegmentation/ext/alglib/interpolation.h"

#include <QPainter>
#include "core/tools/stringutils.h"

namespace campvis {
    const std::string MicroscopyImageSegmentation::loggerCat_ = "CAMPVis.modules.vis.MicroscopyImageSegmentation";

    ContourObject::ContourObject(std::string name)
        : _objectName("objName", "Object Name", name)
        , _color("color", "Color", cgt::vec4(1,1,1,1), cgt::vec4(0,0,0,0), cgt::vec4(1,1,1,1))
        , _visibility("showContour", "Show Contour", true)
    {
    }
    ContourObject::~ContourObject() {
    }

    ContourObject* ContourObject::operator=(const ContourObject& rhs) {
        this->_objectName.setValue(rhs._objectName.getValue());
        this->_color.setValue(rhs._color.getValue());
        this->_points = rhs._points;
        this->_objectsCoordinates = rhs._objectsCoordinates;
        return this;
    }

    void ContourObject::addObject() {
        _objectsCoordinates.push_back(_points);
        _points.clear();
    }

    void ProxyFaceGeometry::addGeometry(FaceGeometry geometry) {
        _geometries.push_back(geometry);
    }
    void ProxyFaceGeometry::render(GLenum mode) const {
        for (size_t i = 0; i < _geometries.size(); i++) {
            _geometries[i].render(mode);
        }
    }

    GenericOption<ContourObject> first[1] = {
        GenericOption<ContourObject>("Select", "Select An Object", ContourObject("Please Select"))
    };

    MicroscopyImageSegmentation::MicroscopyImageSegmentation(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "ve.output", DataNameProperty::WRITE)
        , p_enableScribbling("EnableScribbling", "Enable Scribbling in Slice Views", false)
        , p_seProperties("SliceExtractorProperties", "Slice Extractor Properties")
        , p_vrProperties("VolumeRendererProperties", "Volume Renderer Properties")
        , _tcp(viewportSizeProp)
        , _vr(viewportSizeProp, raycaster)
        , _sliceExtractor(viewportSizeProp)
        , p_rightPaneBlockSize("SliceRenderSize", "Right Pane Block Size", cgt::ivec2(32), cgt::ivec2(0), cgt::ivec2(10000), cgt::ivec2(1))
        , p_leftPaneSize("VolumeRenderSize", "Left Pane Size Size", cgt::ivec2(32), cgt::ivec2(0), cgt::ivec2(10000), cgt::ivec2(1))
        , p_zSize("ZSliceSize", "Main Slice View Size", cgt::ivec2(32), cgt::ivec2(0), cgt::ivec2(10000), cgt::ivec2(1))
        , _xSliceHandler(&_sliceExtractor.p_xSliceNumber)
        , _ySliceHandler(&_sliceExtractor.p_ySliceNumber)
        , _zSliceHandler(&_sliceExtractor.p_zSliceNumber)
        , _windowingHandler(&_sliceExtractor.p_transferFunction)
        , p_paintColor("PaintColor", "Change Color", cgt::vec4(255), cgt::vec4(0.0f), cgt::vec4(255))
        , p_axisScaling("AxisScaling", "Axis Scale", cgt::vec3(1), cgt::vec3(1), cgt::vec3(25), cgt::vec3(1))
        , p_fitToWindow("FitToWindow", "Fit to Window", true)
        , p_scalingFactor("ScalingFactor", "Scaling Factor", 1.f, 0.f, 10.f, .1f, 2)
        , p_offset("Offset", "Offset", cgt::ivec2(0), cgt::ivec2(0), cgt::ivec2(100))
        , p_addObject("addObjectButton", "Add Object")
        , p_deleteObject("deleteObjectButton", "Delete Current Object")
        , p_csvPath("csvPath", "File Name", "", StringProperty::SAVE_FILENAME)
        , p_saveCSV("saveCSV", "Save")
        , p_objectList(0)
        , _mousePressedInRaycaster(false)        
        , _objectNamePrefix("Object ")
        , _objectNameSuffix(0) 

    {
        addProperty(p_inputVolume, INVALID_PROPERTIES);
        addProperty(p_outputImage);


        p_addObject.s_clicked.connect(this, &MicroscopyImageSegmentation::onAddButtonClicked);
        addProperty(p_addObject);
        p_objectList = new GenericOptionProperty<ContourObject>("Object List", "Object List", first, 1);
        p_objectList->s_changed.connect(this, &MicroscopyImageSegmentation::onObjectSelectionChanged);
        addProperty(*p_objectList);
        p_paintColor.s_changed.connect(this, &MicroscopyImageSegmentation::onPaintColorChanged);
        addProperty(p_paintColor);
        p_deleteObject.s_clicked.connect(this, &MicroscopyImageSegmentation::onDeleteButtonClicked);
        addProperty(p_deleteObject);

        _oldScaling = p_axisScaling.getValue();
        addProperty(p_axisScaling);

        p_enableScribbling.setValue(true);
        p_enableScribbling.setVisible(false);
        addProperty(p_enableScribbling, VALID);

        addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);

        p_csvPath.s_changed.connect(this, &MicroscopyImageSegmentation::onCSVFileSelected);
        addProperty(p_csvPath);
        p_saveCSV.s_clicked.connect(this, &MicroscopyImageSegmentation::onSaveCSVButtonClicked);
        addProperty(p_saveCSV);

        p_fitToWindow.addSharedProperty(&_sliceExtractor.p_fitToWindow);
        p_fitToWindow.setValue(false);
        p_scalingFactor.addSharedProperty(&_sliceExtractor.p_scalingFactor);
        p_scalingFactor.setValue(0.3f);
        p_offset.addSharedProperty(&_sliceExtractor.p_offset);
        p_seProperties.addPropertyCollection(_sliceExtractor);

        _sliceExtractor.s_scribblePainted.connect(this, &MicroscopyImageSegmentation::onSliceExtractorScribblePainted);
        _sliceExtractor.p_geometryID.setValue(p_outputImage.getValue() + ".scribbles");

        _sliceExtractor.p_lqMode.setVisible(false);
        _sliceExtractor.p_sourceImageID.setVisible(false);
        _sliceExtractor.p_targetImageID.setVisible(false);
        _sliceExtractor.p_sliceOrientation.setVisible(false);
        _sliceExtractor.p_xSliceColor.setVisible(false);
        _sliceExtractor.p_ySliceColor.setVisible(false);
        _sliceExtractor.p_zSliceColor.setVisible(false);
        _sliceExtractor.p_fitToWindow.setVisible(false);
        _sliceExtractor.p_geometryRenderMode.selectByOption(GL_LINE_STRIP);
        addProperty(p_seProperties, VALID);

        p_vrProperties.addPropertyCollection(_vr);
        _vr.p_lqMode.setVisible(false);
        _vr.p_inputVolume.setVisible(false);
        _vr.p_outputImage.setVisible(false);
        addProperty(p_vrProperties, VALID);

        p_inputVolume.addSharedProperty(&_vr.p_inputVolume);
        p_inputVolume.addSharedProperty(&_sliceExtractor.p_sourceImageID);

        _vr.setViewportSizeProperty(&p_rightPaneBlockSize);
        p_zSize.setVisible(false);
        p_zSize.setValue(p_leftPaneSize.getValue().x < p_leftPaneSize.getValue().y ? 
            cgt::vec2(p_leftPaneSize.getValue().x, p_leftPaneSize.getValue().x) 
            : cgt::vec2(p_leftPaneSize.getValue().y, p_leftPaneSize.getValue().y));

        _sliceExtractor.setViewportSizeProperty(&p_zSize);

        addProperty(p_rightPaneBlockSize, VALID);
        addProperty(p_leftPaneSize, VALID);

        // Event-Handlers
        _tcp.addLqModeProcessor(&_vr);

        editVoxel = false;
        insertNextVoxelAt = -1;
    }

    MicroscopyImageSegmentation::~MicroscopyImageSegmentation() {
        delete p_objectList;
    }

    void MicroscopyImageSegmentation::init() {
        VisualizationProcessor::init();
        _vr.init();
        _sliceExtractor.init();

        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/microscopyimagesegmentation.frag", "");

        _sliceExtractor.s_invalidated.connect(this, &MicroscopyImageSegmentation::onProcessorInvalidated);
        _vr.s_invalidated.connect(this, &MicroscopyImageSegmentation::onProcessorInvalidated);

        _quad = GeometryDataFactory::createQuad(cgt::vec3(0.f), cgt::vec3(1.f), cgt::vec3(0.f), cgt::vec3(1.f));

        // force recalculation of p_sliceRenderSize and p_volumeRenderSize
        onPropertyChanged(_viewportSizeProperty);
    }

    void MicroscopyImageSegmentation::deinit() {
        _vr.deinit();
        _sliceExtractor.deinit();
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        _quad = nullptr; // Object is automatically deleted
    }

    void MicroscopyImageSegmentation::updateResult(DataContainer& data) {
        // launch sub-renderers if necessary
        if (getInvalidationLevel() & SCRIBBLE_INVALID) {
            ContourObject& selectedObject = this->p_objectList->getOptionReference();
            if (!(selectedObject._objectName == first[0]._value._objectName)) {
                // Create a ProxyFaceGeometry and insert all geometry into that
                ProxyFaceGeometry *pg = new ProxyFaceGeometry();
                std::vector<cgt::vec3> vertices;
                std::vector<cgt::vec4> colors;

                size_t size = selectedObject._points.size();

                alglib::real_2d_array xy;
                alglib::pspline2interpolant p;
                size = size > 3? size : size == 0? 0 : 3;
                xy.setlength(size, size);

                for (size_t i = 0; i < selectedObject._points.size(); ++i) {
                    xy[i][0] = selectedObject._points.at(i).x;
                    xy[i][1] = selectedObject._points.at(i).y;
                }

                if(selectedObject._points.size()) {
                    for (size_t i = selectedObject._points.size(); i < 3; ++i) {
                        xy[i][0] = selectedObject._points.at(selectedObject._points.size()-1).x;
                        xy[i][1] = selectedObject._points.at(selectedObject._points.size()-1).y;
                    }
                    //xy[size-1][0] = selectedObject._points.at(0).x;
                    //xy[size-1][1] = selectedObject._points.at(0).y;

                    alglib::pspline2buildperiodic (xy, size, 2, 0, p);
                    float stepSize = 1.0f/ (selectedObject._points.size() * 5);
                    float pos = 0;
                    vertices.clear();
                    colors.clear();
                    for (size_t gg = 0; gg < selectedObject._points.size() * 5; gg++ ) {
                        double x, y, z = selectedObject._points[0].z;
                        alglib::pspline2calc(p, pos, x, y);
                        pos += stepSize;
                        vertices.push_back(cgt::vec3(x, y, z));
                        colors.push_back(selectedObject._color.getValue());
                    }

                    pg->addGeometry(FaceGeometry(vertices, std::vector<cgt::vec3>(), colors));
                }

                for (size_t i = 1; i < p_objectList->getOptionCount(); i++) {
                    for (size_t j = 0; j < p_objectList->getOptionReference(static_cast<int>(i))._objectsCoordinates.size(); j++) {
                        vertices.clear();
                        colors.clear();
                        const std::vector<cgt::ivec3 >& it = p_objectList->getOptionReference(static_cast<int>(i))._objectsCoordinates[j];
                        
                        alglib::real_2d_array xy;
                        alglib::pspline2interpolant p;
                        size = it.size() > 3? it.size() : it.size() == 0? 0 : 3;
                        xy.setlength(size+1, size+1);
                        for (size_t k = 0; k <  it.size(); k++) {
                            xy[k][0] = it[k].x;
                            xy[k][1] = it[k].y;                            
                        }
                        if (it.size()) {
                            for (size_t k = it.size(); k < 3; ++k) {
                                xy[k][0] = it[it.size()-1].x;
                                xy[k][1] = it[it.size()-1].y;
                            }
                            //xy[size-1][0] = it[0].x;
                            //xy[size-1][1] = it[0].y;

                            alglib::pspline2buildperiodic(xy, size, 2, 0, p);
                            float stepSize = 1.0f/ (it.size() * 5);
                            float pos = 0;
                            vertices.clear();
                            colors.clear();
                            for (size_t gg= 0; gg < it.size() * 5; gg++ ) {
                                double x, y, z = it[0].z;
                                alglib::pspline2calc(p, pos, x, y);
                                pos += stepSize;
                                vertices.push_back(cgt::vec3(x, y, z));
                                colors.push_back(p_objectList->getOptionReference(static_cast<int>(i))._color.getValue());
                            }
                            pg->addGeometry(FaceGeometry(vertices, std::vector<cgt::vec3>(), colors));
                        }
                    }
                }
                data.addData(p_outputImage.getValue() + ".scribbles", pg);

                validate(SCRIBBLE_INVALID);

            }

            // force update of slice renderer if necessary
            if (! (getInvalidationLevel() & VR_INVALID))
                invalidate(SLICES_INVALID);
        }
        if (getInvalidationLevel() & AXISSCALE_INVALID) {
            const ImageData* _copy = static_cast<const ImageData*>(data.getData(p_inputVolume.getValue()).getData());
            ImageData* copy = _copy->clone();
            copy->setMappingInformation(ImageMappingInformation(_copy->getSize(), _copy->getMappingInformation().getOffset(), _copy->getMappingInformation().getVoxelSize() * p_axisScaling.getValue() / _oldScaling ));
            data.addData(p_inputVolume.getValue(), copy);
            _oldScaling = p_axisScaling.getValue();
        }
        if (getInvalidationLevel() & VR_INVALID) {
            _tcp.process(data);
            _vr.process(data);
        }
        if (getInvalidationLevel() & SLICES_INVALID) {
            _sliceExtractor.p_sliceOrientation.selectById("x");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".xSlice");
            _sliceExtractor.process(data);

            _sliceExtractor.p_sliceOrientation.selectById("y");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".ySlice");
            _sliceExtractor.process(data);

            _sliceExtractor.p_sliceOrientation.selectById("z");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".zSlice");
            _sliceExtractor.process(data);
        }

        composeFinalRendering(data);

        validate(INVALID_RESULT | VR_INVALID | SLICES_INVALID);
    }

    void MicroscopyImageSegmentation::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == _viewportSizeProperty) {
            p_rightPaneBlockSize.setValue(cgt::ivec2(_viewportSizeProperty->getValue().y / 3, _viewportSizeProperty->getValue().y / 3));
            p_leftPaneSize.setValue(cgt::ivec2(_viewportSizeProperty->getValue().x - _viewportSizeProperty->getValue().y/3, _viewportSizeProperty->getValue().y));

            p_zSize.setValue(p_leftPaneSize.getValue().x < p_leftPaneSize.getValue().y ? 
                cgt::vec2(p_leftPaneSize.getValue().x, p_leftPaneSize.getValue().x) 
                : cgt::vec2(p_leftPaneSize.getValue().y, p_leftPaneSize.getValue().y));
        }
        if (prop == &p_outputImage) {
            _vr.p_outputImage.setValue(p_outputImage.getValue() + ".raycaster");
            _sliceExtractor.p_geometryID.setValue(p_outputImage.getValue() + ".scribbles");
        }
        if (prop == &p_inputVolume) {
            invalidate(VR_INVALID | SLICES_INVALID);
        }
        if (prop == &p_enableScribbling) {
        }
        if (prop == &p_axisScaling) {
            invalidate(VR_INVALID | AXISSCALE_INVALID | SLICES_INVALID);
        }

        VisualizationProcessor::onPropertyChanged(prop);
    }

    void MicroscopyImageSegmentation::composeFinalRendering(DataContainer& data) {
        ScopedTypedData<RenderData> vrImage(data, p_outputImage.getValue() + ".raycaster");
        ScopedTypedData<RenderData> xSliceImage(data, p_outputImage.getValue() + ".xSlice");
        ScopedTypedData<RenderData> ySliceImage(data, p_outputImage.getValue() + ".ySlice");
        ScopedTypedData<RenderData> zSliceImage(data, p_outputImage.getValue() + ".zSlice");

        if (vrImage == 0 && xSliceImage == 0 && ySliceImage == 0 && zSliceImage == 0)
            return;

        FramebufferActivationGuard fag(this);
        createAndAttachColorTexture();
        createAndAttachDepthTexture();

        cgt::TextureUnit colorUnit, depthUnit;
        _shader->activate();

        cgt::vec2 leftPane(p_leftPaneSize.getValue());
        cgt::vec2 blockSize(p_rightPaneBlockSize.getValue());
        cgt::vec2 zsize(p_zSize.getValue());

        cgt::vec2 zsPos(cgt::vec2(0 + (leftPane.x - zsize.x) / 2, 0 + (leftPane.y - zsize.y) / 2));
        cgt::vec2 xsPos(cgt::vec2(leftPane.x, blockSize.y*0));
        cgt::vec2 ysPos(cgt::vec2(leftPane.x, blockSize.y*1));
        cgt::vec2 vrPos(cgt::vec2(leftPane.x, blockSize.y*2));

        _shader->setUniform("_projectionMatrix", cgt::mat4::createOrtho(0, _viewportSizeProperty->getValue().x, _viewportSizeProperty->getValue().y, 0, -1, 1));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (vrImage != 0) {
            decorateRenderProlog(data, _shader);
            _shader->setUniform("_renderBackground", true);

            vrImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(blockSize.x, blockSize.y, .5f)));
            _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(vrPos.x, vrPos.y, 0.f)));
            _quad->render(GL_POLYGON);

            _shader->setUniform("_renderBackground", false);
            decorateRenderEpilog(_shader);
        }
        if (zSliceImage != 0) {
            zSliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(zsize.x, zsize.y, .5f)));
            _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(zsPos.x, zsPos.y, 0.f)));
            _quad->render(GL_POLYGON);
        }
        if (ySliceImage != 0) {
            ySliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(blockSize.x, blockSize.y, .5f)));
            _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(ysPos.x, ysPos.y, 0.f)));
            _quad->render(GL_POLYGON);
        }
        if (xSliceImage != 0) {
            xSliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(blockSize.x, blockSize.y, .5f)));
            _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(xsPos.x, xsPos.y, 0.f)));
            _quad->render(GL_POLYGON);
        }

        _shader->deactivate();
        cgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        data.addData(p_outputImage.getValue(), new RenderData(_fbo));
    }

    void MicroscopyImageSegmentation::onProcessorInvalidated(AbstractProcessor* processor) {
        // make sure to only invalidate ourself if the invalidation is not triggered by us
        // => the _locked state is a trustworthy source for this information :)
        if (! isLocked()) {
            if (processor == &_vr) {
                invalidate(VR_INVALID);
            }
            if (processor == &_sliceExtractor) {
                invalidate(SLICES_INVALID);
            }

            invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void MicroscopyImageSegmentation::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_inputVolume.getValue());
        _sliceExtractor.p_transferFunction.setImageHandle(img.getDataHandle());
        static_cast<TransferFunctionProperty*>(_vr.getNestedProperty("RaycasterProps::TransferFunction"))->setImageHandle(img.getDataHandle());

        if (img != 0) {
            const cgt::svec3& imgSize = img->getSize();
            if (_sliceExtractor.p_xSliceNumber.getMaxValue() != static_cast<int>(imgSize.x) - 1){
                _sliceExtractor.p_xSliceNumber.setMaxValue(static_cast<int>(imgSize.x) - 1);
                _sliceExtractor.p_xSliceNumber.setValue(static_cast<int>(imgSize.x) / 2);
            }
            if (_sliceExtractor.p_ySliceNumber.getMaxValue() != static_cast<int>(imgSize.y) - 1){
                _sliceExtractor.p_ySliceNumber.setMaxValue(static_cast<int>(imgSize.y) - 1);
                _sliceExtractor.p_ySliceNumber.setValue(static_cast<int>(imgSize.y) / 2);
            }
            if (_sliceExtractor.p_zSliceNumber.getMaxValue() != static_cast<int>(imgSize.z) - 1){
                _sliceExtractor.p_zSliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
                _sliceExtractor.p_zSliceNumber.setValue(static_cast<int>(imgSize.z) / 2);
            }

            _tcp.reinitializeCamera(img->getWorldBounds());
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void MicroscopyImageSegmentation::onEvent(cgt::Event* e) {
        cgt::vec2 leftPane(p_leftPaneSize.getValue());
        cgt::vec2 blockSize(p_rightPaneBlockSize.getValue());
        cgt::vec2 zsize(p_zSize.getValue());

        cgt::vec2 zsPos(cgt::vec2(0 + (leftPane.x - zsize.x) / 2, 0 + (leftPane.y - zsize.y) / 2));
        cgt::vec2 xsPos(cgt::vec2(leftPane.x, blockSize.y*0));
        cgt::vec2 ysPos(cgt::vec2(leftPane.x, blockSize.y*1));
        cgt::vec2 vrPos(cgt::vec2(leftPane.x, blockSize.y*2));

        if (typeid(*e) == typeid(cgt::KeyEvent)) {
            cgt::KeyEvent* ke = static_cast<cgt::KeyEvent*>(e);
            ContourObject& selectedObject = this->p_objectList->getOptionReference();
            if (ke->modifiers() & cgt::Event::CTRL && ke->pressed() && ke->keyCode() == 'z') {
                if (selectedObject._points.size() == 0) {
                    if (selectedObject._objectsCoordinates.size() > 0) {
                        selectedObject._points = selectedObject._objectsCoordinates[selectedObject._objectsCoordinates.size() - 1];
                        selectedObject._objectsCoordinates.erase(selectedObject._objectsCoordinates.end() - 1);
                    }
                }
                if (selectedObject._points.size() > 0) 
                    selectedObject._points.erase(selectedObject._points.end()-1);
            }
            else if (ke->keyCode() == cgt::KeyEvent::K_DELETE) {
                if (true == editVoxel) {
                    selectedObject._points.erase(selectedObject._points.begin() + insertNextVoxelAt);
                    editVoxel = false;
                    insertNextVoxelAt = -1;
                    invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
                }
            }
            else if (ke->keyCode() == cgt::KeyEvent::K_UP) {
                _sliceExtractor.p_offset.setValue(_sliceExtractor.p_offset.getValue() - cgt::ivec2(0, 5));
            }
            else if (ke->keyCode() == cgt::KeyEvent::K_DOWN) {
                _sliceExtractor.p_offset.setValue(_sliceExtractor.p_offset.getValue() + cgt::ivec2(0, 5));
            }
            else if (ke->keyCode() == cgt::KeyEvent::K_LEFT) {
                _sliceExtractor.p_offset.setValue(_sliceExtractor.p_offset.getValue() - cgt::ivec2(5, 0));
            }
            else if (ke->keyCode() == cgt::KeyEvent::K_RIGHT) {
                _sliceExtractor.p_offset.setValue(_sliceExtractor.p_offset.getValue() + cgt::ivec2(5, 0));
            }
            invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
        }

        // forward the event to the corresponding event listeners depending on the mouse position
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);

            ContourObject& selectedObject = this->p_objectList->getOptionReference();

            // we're currently on the slice view (left-hand) side and not in the process of changing the camera trackball
            if (!_mousePressedInRaycaster && (me->x() <= leftPane.x || me->y() >= blockSize.y)) {
                // Mouse wheel has changed -> cycle slices
                if (me->action() == cgt::MouseEvent::WHEEL && !(me->modifiers() & cgt::Event::CTRL)) {
                    if (me->y() >= blockSize.y*2 && me->x() >= leftPane.x)
                        _xSliceHandler.onEvent(e);
                    else if (me->y() >= blockSize.y && me->x() >= leftPane.x)
                        _ySliceHandler.onEvent(e);
                    else
                        _zSliceHandler.onEvent(e);
                }
                // CTRL+wheel -> adjust zooming
                else if (me->action() == cgt::MouseEvent::WHEEL && me->modifiers() & cgt::Event::CTRL) {
                    if (_sliceExtractor.p_fitToWindow.getValue() == false) {
                        if (me->button() & cgt::MouseEvent::MOUSE_WHEEL_DOWN)
                            _sliceExtractor.p_scalingFactor.setValue(_sliceExtractor.p_scalingFactor.getValue() * 0.9f);
                        else 
                            _sliceExtractor.p_scalingFactor.setValue(_sliceExtractor.p_scalingFactor.getValue() * 1.11f);
                    }
                }
                else if (p_enableScribbling.getValue() ) {
                    if (me->action() == cgt::MouseEvent::PRESSED) {
                        if (me->button() & cgt::MouseEvent::MOUSE_BUTTON_RIGHT) {
                            selectedObject.addObject();
                            //_sliceExtractor.onEvent(me);
                            invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
                        }
                    }

                    // lock this processor, so that the slice orientation's setting does not change
                    AbstractProcessor::ScopedLock lock(this);

                    if (me->y() <= blockSize.y * 2 && me->x() >= leftPane.x) {
                        _sliceExtractor.p_sliceOrientation.selectByOption(SliceExtractor::XZ_PLANE);//xz
                        cgt::MouseEvent adjustedMe(
                            me->x() - p_leftPaneSize.getValue().x, me->y() - p_rightPaneBlockSize.getValue().y,
                            me->action(), me->modifiers(), me->button(),
                            p_rightPaneBlockSize.getValue()
                            );
                        _sliceExtractor.setViewportSizeProperty(&p_rightPaneBlockSize);
                        _sliceExtractor.onEvent(&adjustedMe);
                        _sliceExtractor.setViewportSizeProperty(&p_zSize);
                    }
                    else if (me->y() >= blockSize.y * 2 && me->x() >= leftPane.x) {
                        _sliceExtractor.p_sliceOrientation.selectByOption(SliceExtractor::YZ_PLANE);//yz
                        cgt::MouseEvent adjustedMe(
                            me->x() - p_leftPaneSize.getValue().x, me->y() - p_rightPaneBlockSize.getValue().y *2,
                            me->action(), me->modifiers(), me->button(),
                            p_rightPaneBlockSize.getValue()
                            );

                        _sliceExtractor.setViewportSizeProperty(&p_rightPaneBlockSize);
                        _sliceExtractor.onEvent(&adjustedMe);
                        _sliceExtractor.setViewportSizeProperty(&p_zSize);
                    }
                    else if (me->x() <  (leftPane.x - zsize.x) / 2 || me->y() <  (leftPane.y - zsize.y) / 2
                        || me->x() >  (leftPane.x - zsize.x) / 2 + zsize.x || me->y() >  (leftPane.y - zsize.y) / 2 +zsize.y){
                            // action in blank space
                    }
                    else {
                        // If ALT is pressed mark for edit. next scribble is edit scribble. the location of insert is insertNextVoxelAt
                        if (me->modifiers() & cgt::Event::ALT)
                            editVoxel = true;

                        _sliceExtractor.p_sliceOrientation.selectByOption(SliceExtractor::XY_PLANE);
                        cgt::MouseEvent adjustedMe(
                            me->x() -  (leftPane.x - zsize.x) / 2, me->y() -  (leftPane.y - zsize.y) / 2,
                            me->action(), me->modifiers(), me->button(),
                            p_zSize.getValue()
                            );
                        _sliceExtractor.onEvent(&adjustedMe);
                    }
                }
            }
            else {
                // raycasting trackball navigation
                if (me->action() == cgt::MouseEvent::PRESSED)
                    _mousePressedInRaycaster = true;
                else if (me->action() == cgt::MouseEvent::RELEASED)
                    _mousePressedInRaycaster = false;

                cgt::MouseEvent adjustedMe(
                    me->x() - p_leftPaneSize.getValue().x,
                    me->y(),
                    me->action(),
                    me->modifiers(),
                    me->button(),
                    //me->viewport() - cgt::ivec2(p_sliceRenderSize.getValue().x, 0)
                    //me->viewport() - p_volumeRenderSize.getValue());
                    p_rightPaneBlockSize.getValue());
                _tcp.onEvent(&adjustedMe);
            }
        }
    }

    void MicroscopyImageSegmentation::onSliceExtractorScribblePainted(cgt::vec3 voxel) {
        ContourObject& selectedObject = this->p_objectList->getOptionReference();

        if (!(selectedObject._objectName == first[0]._value._objectName)) {
            if (true == editVoxel && insertNextVoxelAt == -1) {
                unsigned long dist = -1;
                if (!selectedObject._points.size()) {
                    editVoxel = false;
                    return;
                }

                for (size_t i = 0; i < selectedObject._points.size(); i++) {
                    unsigned long dist2 = distanceSqr(selectedObject._points[i], voxel);
                    if (dist > dist2) {
                        insertNextVoxelAt = static_cast<int>(i);
                        dist = dist2;
                    }
                }
            }
            else if (true == editVoxel) {
                selectedObject._points.erase(selectedObject._points.begin() + insertNextVoxelAt);
                selectedObject._points.insert(selectedObject._points.begin() + insertNextVoxelAt, voxel);
                editVoxel = false;
                insertNextVoxelAt = -1;
                invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
            }
            else {
                selectedObject._points.push_back(voxel);
                invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
            }
        }
    }

    void MicroscopyImageSegmentation::onAddButtonClicked() {
        std::string name = _objectNamePrefix;
        name += StringUtils::toString(++_objectNameSuffix, 2, '0');
        ContourObject obj(name);
        obj._color.setValue(cgt::vec4(
            static_cast<float>(0.9 - 0.2 * (_objectNameSuffix%3) + 0.15 *(_objectNameSuffix/3)),
            static_cast<float>(0.2 + 0.2 * (_objectNameSuffix%3) - 0.15 *(_objectNameSuffix/3)), 
            static_cast<float>(0.5 + 0.2 * (_objectNameSuffix%3) - 0.15 *(_objectNameSuffix/3)), 1));
        p_objectList->addOption(GenericOption<ContourObject>(name, name, obj));
        p_objectList->selectById(name);
    }

    void MicroscopyImageSegmentation::onDeleteButtonClicked() {
        p_objectList->removeCurrent();
        invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
    }

    void MicroscopyImageSegmentation::onPaintColorChanged(const AbstractProperty *prop) {
        ContourObject &obj = this->p_objectList->getOptionReference();
        obj._color.setValue(p_paintColor.getValue());
        this->p_objectList->updateCurrent(obj);

        invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
    }
    void MicroscopyImageSegmentation::onObjectSelectionChanged (const AbstractProperty *prop) {
        const ContourObject &obj = this->p_objectList->getOptionValue();
        this->p_paintColor.setValue(obj._color.getValue());

        invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
    }

    double MicroscopyImageSegmentation::distanceSqr(cgt::vec3 src, cgt::vec3 dest) {
        return (src.x - dest.x)*(src.x - dest.x) 
            + (src.y - dest.y)*(src.y - dest.y) 
            + (src.z - dest.z)*(src.z - dest.z);
    }

    void MicroscopyImageSegmentation::onCSVFileSelected(const AbstractProperty *prop) {
        onSaveCSVButtonClicked();
    }
    void MicroscopyImageSegmentation::onSaveCSVButtonClicked() {
        std::ofstream csvFile;
        csvFile.open(p_csvPath.getValue());

        csvFile << "ObjectNo,ContourNo,X,Y,Z\n";
        if (csvFile.is_open()) {
            for (size_t i = 1; i < p_objectList->getOptionCount(); i++) {
                csvFile << i<<",,,,\n";
                for (size_t j = 0; j < p_objectList->getOptionReference(static_cast<int>(i))._objectsCoordinates.size(); j++) {
                    const std::vector<cgt::ivec3 >& it = p_objectList->getOptionReference(static_cast<int>(i))._objectsCoordinates[j];

                    csvFile << "," <<j << ",,,\n";
                    for (size_t k = 0; k <  it.size(); k++) {
                        csvFile << ",," << it[k].x << "," << it[k].y << "," << it[k].z <<",\n";
                    }                    
                }
            }
        }
        csvFile.close();
    }
}

