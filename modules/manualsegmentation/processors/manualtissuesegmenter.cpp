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

#include "manualtissuesegmenter.h"
#include "cgt/bounds.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include "core/tools/interval.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/renderdata.h"

#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/facegeometry.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

#ifdef CAMPVIS_HAS_MODULE_ITK
#include <itkPointSet.h>
#include <itkBSplineScatteredDataPointSetToImageFilter.h>
#include <itkBSplineControlPointImageFunction.h>
#include <itkExceptionObject.h>
#endif


#include <fstream>

namespace campvis {
    const std::string ManualTissueSegmenter::loggerCat_ = "CAMPVis.modules.vis.ManualTissueSegmenter";

    ManualTissueSegmenter::ManualTissueSegmenter(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceFioID", "Input Tracked US File IO", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_frameNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_fullWidth("FullWidth", "Full Width", true)
        , p_splineOrder("SplineOrder", "Spline Order", 3, 1, 9)
        , p_NumControlPoints("NumControlPoints", "Number of Control Points", 4, 1, 10)
        , p_NumLevels("NumLevels", "Number of Levels", 10, 1, 15)
        , p_computeSamples("ComputeSamples", "Compute Samples")
        , p_showSamples("ShowSamples", "Show Samples", false)
        , p_sampleFile("SampleFile", "Sample File", "", StringProperty::SAVE_FILENAME)
        , p_saveSamples("SaveSamples", "Save Samples")
        , p_controlpointFile("ControlpointFile", "Control Point File", "C:\\temp\\us_small.cps", StringProperty::SAVE_FILENAME)
        , p_saveCPs("SaveCPs", "Save Control Points")
        , p_loadCPs("LoadCPs", "Load Control Points")
        , p_activeLayer("ActiveLayer", "Active Layer", 0, 0, 0)
        , p_addLayer("AddLayer", "Add Layer")
        , p_exportToLabelImage("ExportToLabelImage", "Export to Label Image")
        , _shader(0)
        , _currentImage(0)
        , _mousePressed(false)
        , _mouseDownPosition(0)
        , _currentControlPoint(0)
        , _currentBounds(0)
    {
        addProperty(p_sourceImageID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetImageID);
        addProperty(p_frameNumber);
        addProperty(p_transferFunction);

        addProperty(p_fullWidth);
        addProperty(p_splineOrder);
        addProperty(p_NumControlPoints);
        addProperty(p_NumLevels);

        addProperty(p_computeSamples, VALID);
        addProperty(p_showSamples);
        addProperty(p_sampleFile, VALID);
        addProperty(p_saveSamples, VALID);

        addProperty(p_controlpointFile, VALID);
        addProperty(p_saveCPs, VALID);
        addProperty(p_loadCPs, VALID);

        addProperty(p_activeLayer);
        addProperty(p_addLayer, VALID);

        addProperty(p_exportToLabelImage, INVALID_RESULT | FIRST_FREE_TO_USE_INVALIDATION_LEVEL);

        p_computeSamples.s_clicked.connect(this, &ManualTissueSegmenter::computeSamples);
        p_saveSamples.s_clicked.connect(this, &ManualTissueSegmenter::saveSamples);
        p_saveCPs.s_clicked.connect(this, &ManualTissueSegmenter::saveControlPoints);
        p_loadCPs.s_clicked.connect(this, &ManualTissueSegmenter::loadControlPoints);
        p_addLayer.s_clicked.connect(this, &ManualTissueSegmenter::onAddLayer);
    }

    ManualTissueSegmenter::~ManualTissueSegmenter() {

    }

    void ManualTissueSegmenter::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/manualsegmentation/glsl/manualtissuesegmenter.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _controlPoints.push_back(std::map< int, std::vector< ControlPoint > >());
        _segmentation.addLayer(TissueSegmentation::Layer());
    }

    void ManualTissueSegmenter::deinit() {
        ShdrMgr.dispose(_shader);
        _currentImage = DataHandle(nullptr);

        VisualizationProcessor::deinit();
    }

    void ManualTissueSegmenter::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                _currentImage = img.getDataHandle();
                tgt::vec3 imgSize(img->getSize());

                if (getInvalidationLevel() & FIRST_FREE_TO_USE_INVALIDATION_LEVEL) {
                    const tgt::svec3& size = img->getSize();
                    ImageData* labelImage = new ImageData(3, size, 1);
                    GenericImageRepresentationLocal<uint8_t, 1>* rep = GenericImageRepresentationLocal<uint8_t, 1>::create(labelImage, 0);
                    labelImage->setMappingInformation(img->getParent()->getMappingInformation());

                    for (size_t z = 0; z < size.z; ++z) {
                        for (size_t x = 0; x < size.x; ++x) {
                            size_t yStart = 0;

                            for (size_t layer = 0; layer < _segmentation.getNumLayers(); ++layer) {
                                size_t yEnd = static_cast<size_t>(tgt::clamp(_segmentation.getLayer(layer)._frames[z]._points[x].y, 0.f, static_cast<float>(size.y)));

                                for (size_t y = yStart; y < yEnd; ++y) {
                                    rep->setElement(tgt::svec3(x, y, z), 1 << layer);
                                }

                                yStart = yEnd;
                            }

                            for (size_t y = yStart; y < size.y; ++y) {
                                rep->setElement(tgt::svec3(x, y, z), 1 << _segmentation.getNumLayers());
                            }
                        }
                    }

                    // write vessel
                    if (_segmentation._centerlines.size() >= size.z) {
                        for (size_t z = 0; z < size.z; ++z) {
                            tgt::vec2 centerpoint = _segmentation._centerlines[z].xy();
                            tgt::vec2 radius = _segmentation._centerlines[z].zw();

                            tgt::vec2 start = tgt::clamp(centerpoint - radius, tgt::vec2(0.f), tgt::vec2(size.xy()));
                            tgt::vec2 end = tgt::clamp(centerpoint + radius, tgt::vec2(0.f), tgt::vec2(size.xy()));

                            for (size_t x = start.x; x < end.x; ++x) {
                                for (size_t y = start.y; y < end.y; ++y) {
                                    rep->getElement(tgt::svec3(x, y, z)) += 1 << (_segmentation.getNumLayers() + 1);
                                }
                            }                            
                        }
                    }

                    data.addData("LabelImage", labelImage);
                    validate(FIRST_FREE_TO_USE_INVALIDATION_LEVEL);
                }

                // prepare OpenGL
                _shader->activate();

                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);
                float zTexCoord = static_cast<float>(p_frameNumber.getValue())/static_cast<float>(imgSize.z) + .5f/static_cast<float>(imgSize.z);
                _shader->setUniform("_zTexCoord", zTexCoord);

                // render slice
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

                tgt::ivec2 renderTargetSize = getEffectiveViewportSize();

                glPushAttrib(GL_ALL_ATTRIB_BITS);
                if (p_showSamples.getValue()) {
                    LGL_ERROR;
                    glEnable(GL_BLEND);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    glMatrixMode(GL_PROJECTION);
                    glPushMatrix();
                    glLoadIdentity();
                    glOrtho(0, img->getSize().x, 0, img->getSize().y, -1, 1);

                    {
                        glPointSize(6.f);
                        glColor4f(1.f, 1.f, 1.f, .75f);
                        glBegin(GL_LINE_STRIP);
                        const TissueSegmentation::Layer& l  = _segmentation.getLayer(p_activeLayer.getValue());
                        if (l._frames.size() > p_frameNumber.getValue()) {
                            for (size_t i = 0; i < l._frames[p_frameNumber.getValue()]._points.size(); ++i) {
                                const tgt::vec2& pt = l._frames[p_frameNumber.getValue()]._points[i];
                                glVertex2fv(pt.elem);
                            }
                        }
                        glEnd();
                        glPointSize(1.f);
                    }
                    {
                        if (_segmentation._centerlines.size() >= p_frameNumber.getValue()) {
                            glColor4f(1.f, .5f, 0.f, 1.f);
                            tgt::vec2 llf = (_segmentation._centerlines[p_frameNumber.getValue()].xy() - _segmentation._centerlines[p_frameNumber.getValue()].zw());
                            tgt::vec2 urb = (_segmentation._centerlines[p_frameNumber.getValue()].xy() + _segmentation._centerlines[p_frameNumber.getValue()].zw());

                            glBegin(GL_LINE_LOOP);
                                glVertex2f(llf.x, llf.y);
                                glVertex2f(llf.x, urb.y);
                                glVertex2f(urb.x, urb.y);
                                glVertex2f(urb.x, llf.y);
                            glEnd();

                        }
                    }

                    glPopMatrix();
                    LGL_ERROR;
                }
                else {
                    {
                        typedef std::map< int, std::vector< float > > MapType;
                        MapType::const_iterator it = _splines.find(p_frameNumber.getValue());
                        if (it != _splines.end()) {
                            LGL_ERROR;
                            glEnable(GL_BLEND);
                            glMatrixMode(GL_MODELVIEW);
                            glLoadIdentity();
                            glMatrixMode(GL_PROJECTION);
                            glPushMatrix();
                            glLoadIdentity();
                            glOrtho(0, renderTargetSize.x, 0, renderTargetSize.y, -1, 1);
                            glPointSize(6.f);
                            glColor4f(1.f, 1.f, 1.f, .75f);
                            glBegin(GL_LINE_STRIP);
                            float stepsize = 1.f / it->second.size();
                            for (size_t i = 0; i < it->second.size(); ++i) {
                                tgt::vec2 tmp(stepsize*i, it->second[i]);
                                tmp *= tgt::vec2(renderTargetSize);
                                glVertex2fv(tmp.elem);
                            }
                            glEnd();
                            glPointSize(1.f);
                            glPopMatrix();
                            LGL_ERROR;
                        }
                    }
                    {
                        typedef std::map< int, std::vector< ControlPoint > > MapType;
                        MapType::const_iterator it = _controlPoints[p_activeLayer.getValue()].find(p_frameNumber.getValue());
                        if (it != _controlPoints[p_activeLayer.getValue()].end()) {
                            LGL_ERROR;
                            glEnable(GL_BLEND);
                            glMatrixMode(GL_MODELVIEW);
                            glLoadIdentity();
                            glMatrixMode(GL_PROJECTION);
                            glPushMatrix();
                            glLoadIdentity();
                            glOrtho(0, renderTargetSize.x, 0, renderTargetSize.y, -1, 1);
                            glPointSize(8.f);
                            glColor4f(1.f, 1.f, 1.f, .5f);
                            glBegin(GL_POINTS);
                            for (size_t i = 0; i < it->second.size(); ++i) {
                                tgt::vec2 tmp = it->second[i]._pixel * tgt::vec2(renderTargetSize);
                                glVertex2fv(tmp.elem);
                            }
                            glEnd();
                            glPointSize(1.f);
                            glPopMatrix();
                            LGL_ERROR;
                        }
                    }
                    {
                        typedef std::map< int, tgt::vec4 > MapType;
                        MapType::const_iterator it = _vesselBounds.find(p_frameNumber.getValue());
                        if (it != _vesselBounds.end()) {
                            LGL_ERROR;
                            glEnable(GL_BLEND);
                            glMatrixMode(GL_MODELVIEW);
                            glLoadIdentity();
                            glMatrixMode(GL_PROJECTION);
                            glPushMatrix();
                            glLoadIdentity();
                            glOrtho(0, renderTargetSize.x, 0, renderTargetSize.y, -1, 1);

                            glColor4f(1.f, .5f, 0.f, 1.f);
                            tgt::vec2 llf = (it->second.xy() - it->second.zw()) * tgt::vec2(renderTargetSize);
                            tgt::vec2 urb = (it->second.xy() + it->second.zw()) * tgt::vec2(renderTargetSize);
                            glBegin(GL_LINE_LOOP);
                                glVertex2f(llf.x, llf.y);
                                glVertex2f(llf.x, urb.y);
                                glVertex2f(urb.x, urb.y);
                                glVertex2f(urb.x, llf.y);
                            glEnd();

                            glPopMatrix();
                            LGL_ERROR;
                        }
                    }
                }
                
                glPopAttrib();
                LGL_ERROR;

                data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }
    }

    void ManualTissueSegmenter::updateProperties(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                p_frameNumber.setMaxValue(img->getSize().z - 1);
            }
        }
    }

    void ManualTissueSegmenter::onEvent(tgt::Event* e) {
        if (tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e)) {
            tgt::vec2 renderTargetSize(_viewportSizeProperty->getValue());
            tgt::vec2 texPos(static_cast<float>(me->x()) / renderTargetSize.x, 1.f - (static_cast<float>(me->y()) / renderTargetSize.y));

            if (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT && me->action() == tgt::MouseEvent::PRESSED && me->modifiers() & tgt::Event::CTRL) {
                _mousePressed = true;
                _mouseDownPosition = tgt::ivec2(me->x(), me->y());

                std::vector<ControlPoint>& cps = _controlPoints[p_activeLayer.getValue()][p_frameNumber.getValue()];
                cps.push_back(ControlPoint());
                _currentControlPoint = &cps.back();
                _currentBounds = 0;
                _currentControlPoint->_pixel = texPos;

                invalidate(INVALID_RESULT);
                e->ignore();
            }
            else if (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT && me->action() == tgt::MouseEvent::PRESSED && me->modifiers() & tgt::Event::ALT) {
                _mousePressed = true;
                _mouseDownPosition = tgt::ivec2(me->x(), me->y());

                _vesselBounds[p_frameNumber.getValue()] = tgt::vec4(texPos, 1.f, 1.f);
                _currentBounds = &_vesselBounds[p_frameNumber.getValue()];
                _currentControlPoint = 0;

                invalidate(INVALID_RESULT);
                e->ignore();
            }
            else if (me->action() == tgt::MouseEvent::PRESSED && !(me->modifiers() & tgt::Event::CTRL)) {
                _mouseDownPosition = tgt::ivec2(me->x(), me->y());

                std::vector<ControlPoint>& cps = _controlPoints[p_activeLayer.getValue()][p_frameNumber.getValue()];
                for (size_t i = 0; i < cps.size(); ++i) {
                    tgt::vec2 tmp = cps[i]._pixel * tgt::vec2(renderTargetSize);
                    if (std::abs(me->x() - tmp.x) < 6 && std::abs(renderTargetSize.y - me->y() - tmp.y) < 6) {
                        if (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) {
                            _currentControlPoint = &cps[i];
                            _mousePressed = true;
                        }
                        else if (me->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) {
                            cps.erase(cps.begin() + i);
                            computeSpline();
                            invalidate(INVALID_RESULT);
                        }
                    }
                }

            }
            else if (_mousePressed && me->action() == tgt::MouseEvent::RELEASED) {
                _mousePressed = false;
                _currentControlPoint = 0;
                _currentBounds = 0;

                computeSpline();
                invalidate(INVALID_RESULT);
                e->ignore();
            }
            else if (_mousePressed && me->action() == tgt::MouseEvent::MOTION) {
                if (_currentBounds != 0) {
                    tgt::ivec2 currentPosition(me->x(), me->y());
                    tgt::ivec2 delta = currentPosition - _mouseDownPosition;
                    _currentBounds->z = std::abs(delta.x / renderTargetSize.x);
                    _currentBounds->w = std::abs(delta.y / renderTargetSize.y);
                    e->ignore();
                    invalidate(INVALID_RESULT);
                }
                else if (_currentControlPoint != 0) {
                    tgt::ivec2 currentPosition(me->x(), me->y());
                    tgt::ivec2 delta = currentPosition - _mouseDownPosition;
                    _currentControlPoint->_pixel = texPos;

                    computeSpline();
                    e->ignore();
                }
            }
        }
    }

// ================================================================================================

    void ManualTissueSegmenter::computeSpline() {
#ifdef CAMPVIS_HAS_MODULE_ITK
        if (_currentImage.getData() == nullptr)
            return;

        // class invariant: _currentImage has nullptr or ptr to ImageData => static cast is safe
        const ImageData* image = static_cast<const ImageData*>(_currentImage.getData());

        const unsigned int ParametricDimension = 1;
        const unsigned int DataDimension = 1;

        typedef double RealType;
        typedef itk::Vector<RealType, DataDimension> VectorType;
        typedef itk::Image<VectorType, ParametricDimension> ImageType;  

        typedef itk::PointSet<VectorType, ParametricDimension> PointSetType;
        PointSetType::Pointer pointSet = PointSetType::New();  

        // Sample the points
        typedef std::map< int, std::vector< ControlPoint > > MapType;
        MapType::const_iterator it = _controlPoints[p_activeLayer.getValue()].find(p_frameNumber.getValue());

        if (it != _controlPoints[p_activeLayer.getValue()].end()) {
            for (size_t i = 0; i < it->second.size(); ++i) {
                PointSetType::PointType point;
                point[0] = it->second[i]._pixel.x;
                pointSet->SetPoint(i, point);

                VectorType V;
                V[0] = it->second[i]._pixel.y;
                pointSet->SetPointData(i, V);
            }
        }

        // Instantiate the filter and set the parameters
        typedef itk::BSplineScatteredDataPointSetToImageFilter<PointSetType, ImageType>  FilterType;
        FilterType::Pointer filter = FilterType::New();

        // Define the parametric domain
        ImageType::SpacingType spacing;  
        spacing.Fill( 1.f / image->getSize().x);
        ImageType::SizeType size;  
        size.Fill( 1.f / spacing[0] );
        ImageType::PointType origin;  
        origin.Fill( 0.0 );

        filter->SetSize( size );
        filter->SetOrigin( origin );
        filter->SetSpacing( spacing );
        filter->SetInput( pointSet );

        filter->SetSplineOrder( p_splineOrder.getValue() );  
        FilterType::ArrayType ncps;
        ncps.Fill( p_NumControlPoints.getValue() );  
        filter->SetNumberOfControlPoints( ncps );
        filter->SetNumberOfLevels( p_NumLevels.getValue() );
        filter->SetGenerateOutputImage( false );

        try 
        {
            filter->Update();

            typedef itk::BSplineControlPointImageFunction< ImageType, double > EvalFunctionType;
            EvalFunctionType::Pointer function = EvalFunctionType::New();

            function->SetSplineOrder(filter->GetSplineOrder());
            function->SetOrigin(filter->GetOrigin());
            function->SetSpacing(filter->GetSpacing());
            function->SetSize(filter->GetSize());
            function->SetInputImage(filter->GetPhiLattice());

            std::vector<float> tmp;

            float inc = 2.f / image->getSize().x;
            for (float t = 0.f; t <= 1.0+1e-10; t += inc)
            {
                PointSetType::PointType point;
                point[0] = t;

                VectorType v; 
                v = function->Evaluate(point);
                tmp.push_back(v[0]);
            }

            _splines[p_frameNumber.getValue()] = tmp;

        }
        catch (itk::ExceptionObject& e) {
            LERROR(e.what());
        }

        invalidate(INVALID_RESULT);
#endif
    }

// ================================================================================================

    void ManualTissueSegmenter::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_NumControlPoints || prop == &p_splineOrder || prop == &p_NumLevels) {
            computeSpline();
        }
        else
            VisualizationProcessor::onPropertyChanged(prop);
    }

    void ManualTissueSegmenter::computeSamples() {
#ifdef CAMPVIS_HAS_MODULE_ITK
        if (_currentImage.getData() == nullptr)
            return;

        // class invariant: _currentImage has nullptr or ptr to ImageData => static cast is safe
        const ImageData* image = static_cast<const ImageData*>(_currentImage.getData());

        const unsigned int ParametricDimension = 2;
        const unsigned int DataDimension = 1;

        typedef double RealType;
        typedef itk::Vector<RealType, DataDimension> VectorType;
        typedef itk::Image<VectorType, ParametricDimension> ImageType;  

        for (size_t layer = 0; layer < _controlPoints.size(); ++layer) {
            typedef itk::PointSet<VectorType, ParametricDimension> PointSetType;
            PointSetType::Pointer pointSet = PointSetType::New();  

            // Sample the points
            typedef std::map< int, std::vector< ControlPoint > > MapType;
            for (MapType::const_iterator it = _controlPoints[layer].begin(); it != _controlPoints[layer].end(); ++it) {
                for (size_t i = 0; i < it->second.size(); ++i) {
                    size_t j = pointSet->GetNumberOfPoints();

                    PointSetType::PointType point;
                    point[0] = it->second[i]._pixel.x;
                    point[1] = static_cast<float>(it->first) / image->getSize().z;
                    pointSet->SetPoint(j, point);

                    VectorType V;
                    V[0] = it->second[i]._pixel.y;
                    pointSet->SetPointData(j, V);
                }
            }

            // Instantiate the filter and set the parameters
            typedef itk::BSplineScatteredDataPointSetToImageFilter<PointSetType, ImageType>  FilterType;
            FilterType::Pointer filter = FilterType::New();

            // Define the parametric domain
            ImageType::SpacingType spacing;  
            spacing[0] = 1.f / image->getSize().x;
            spacing[1] = 1.f / image->getSize().z;
            ImageType::SizeType size;  
            size[0] = 1.f / spacing[0];
            size[1] = image->getSize().z;
            ImageType::PointType origin;  
            origin.Fill( 0.0 );

            filter->SetSize( size );
            filter->SetOrigin( origin );
            filter->SetSpacing( spacing );
            filter->SetInput( pointSet );

            filter->SetSplineOrder( p_splineOrder.getValue() );  
            FilterType::ArrayType ncps;
            ncps.Fill( p_NumControlPoints.getValue() );  
            filter->SetNumberOfControlPoints( ncps );
            filter->SetNumberOfLevels( p_NumLevels.getValue() );
            filter->SetGenerateOutputImage( false );

            try 
            {
                filter->Update();

                typedef itk::BSplineControlPointImageFunction< ImageType, double > EvalFunctionType;
                EvalFunctionType::Pointer function = EvalFunctionType::New();

                function->SetSplineOrder(filter->GetSplineOrder());
                function->SetOrigin(filter->GetOrigin());
                function->SetSpacing(filter->GetSpacing());
                function->SetSize(filter->GetSize());
                function->SetInputImage(filter->GetPhiLattice());

                float inc = 1.f / image->getSize().x;
                TissueSegmentation::Layer l;
                l._frames.resize(image->getSize().z);
                for (int i = 0; i < image->getSize().z; ++i) {

                    for (float t = 0.f; t <= 1.0+1e-10; t += inc)
                    {
                        PointSetType::PointType point;
                        point[0] = t;
                        point[1] = static_cast<float>(i) / image->getSize().z;

                        VectorType v; 
                        v = function->Evaluate(point);
                        l._frames[i]._points.push_back(tgt::vec2(t * image->getSize().x, v[0] * image->getSize().y));

                        EvalFunctionType::GradientType gt = function->EvaluateGradient(point);
                        tgt::vec2 g(gt[0][0], gt[0][1]);
                        l._frames[i]._gradients.push_back(g);
                    }
                }
                _segmentation.setLayer(layer, l);
            }
            catch (itk::ExceptionObject& e) {
                LERROR(e.what());
            }
        }

        // vessel centerlines
        _segmentation._centerlines.clear();
        if (! _vesselBounds.empty()) {
            _segmentation._centerlines.resize(image->getSize().z, tgt::vec4(-1.f));

            for (int z = 0; z < image->getSize().z; ++z) {
                std::map< int, tgt::vec4 >::iterator ub = _vesselBounds.lower_bound(z);

                if (ub == _vesselBounds.end()) {
                    --ub;
                    _segmentation._centerlines[z] = ub->second * tgt::vec4(image->getSize().xy(), image->getSize().x, image->getSize().y);
                }
                else if (ub->first == z) {
                    _segmentation._centerlines[z] = ub->second * tgt::vec4(image->getSize().xy(), image->getSize().x, image->getSize().y);
                }
                else if (ub != _vesselBounds.begin()) {
                    int ee = ub->first;
                    tgt::vec4 end = ub->second * tgt::vec4(image->getSize().xy(), image->getSize().x, image->getSize().y);

                    --ub;
                    tgt::vec4 start = ub->second * tgt::vec4(image->getSize().xy(), image->getSize().x, image->getSize().y);
                    int ss = ub->first;

                    float fraction = static_cast<float>(z - ss) / static_cast<float>(ee - ss);
                    _segmentation._centerlines[z] = start + fraction * (end-start);
                }
                else {
                    _segmentation._centerlines[z] = ub->second * tgt::vec4(image->getSize().xy(), image->getSize().x, image->getSize().y);
                }
            }
        }

        LDEBUG("done.");
#endif


    }

    void ManualTissueSegmenter::saveSamples() {
        std::ofstream f;
        f.open(p_sampleFile.getValue(), std::ios::binary | std::ios::out);
        _segmentation.saveToStream(f);
        f.close();
    }

    void ManualTissueSegmenter::saveControlPoints() {
        std::ofstream f;
        f.open(p_controlpointFile.getValue(), std::ios::binary | std::ios::out);

        size_t numLayers = _controlPoints.size();
        f.write(reinterpret_cast<char*>(&numLayers), sizeof(size_t));

        for (size_t layer = 0; layer < numLayers; ++layer) {
            size_t count = _controlPoints[layer].size();
            f.write(reinterpret_cast<char*>(&count), sizeof(size_t));

            typedef std::map< int, std::vector< ControlPoint > > MapType;
            for (MapType::iterator it = _controlPoints[layer].begin(); it != _controlPoints[layer].end(); ++it) {
                int frame = it->first;
                f.write(reinterpret_cast<char*>(&frame), sizeof(int));

                size_t numSamples = it->second.size();
                f.write(reinterpret_cast<char*>(&numSamples), sizeof(size_t));

                if (numSamples > 0)
                    f.write(reinterpret_cast<char*>(&(it->second.front())), numSamples * sizeof(ControlPoint));
            }
        }

        size_t numVessels = _vesselBounds.size();
        f.write(reinterpret_cast<char*>(&numVessels), sizeof(size_t));

        typedef std::map< int, tgt::vec4 > MapType;
        for (MapType::iterator it = _vesselBounds.begin(); it != _vesselBounds.end(); ++it) {
            int frame = it->first;
            f.write(reinterpret_cast<char*>(&frame), sizeof(int));
            f.write(reinterpret_cast<char*>(&it->second), sizeof(tgt::vec4));
        }

        f.close();
    }

    void ManualTissueSegmenter::loadControlPoints() {
        _controlPoints.clear();
        _segmentation.clear();
        
        std::ifstream f;
        f.open(p_controlpointFile.getValue(), std::ios::binary | std::ios::in);

        size_t numLayers = 0;
        f.read(reinterpret_cast<char*>(&numLayers), sizeof(size_t));

        for (size_t layer = 0; layer < numLayers; ++layer) {
            size_t count = 0;
            f.read(reinterpret_cast<char*>(&count), sizeof(size_t));
            std::map< int, std::vector< ControlPoint > > currentLayer;

            for (size_t i = 0; i < count; ++i) {
                int frame = 0;
                f.read(reinterpret_cast<char*>(&frame), sizeof(int));

                size_t numSamples = 0;
                f.read(reinterpret_cast<char*>(&numSamples), sizeof(size_t));

                std::vector<ControlPoint> tmp;
                tmp.resize(numSamples);

                if (numSamples > 0)
                    f.read(reinterpret_cast<char*>(&(tmp.front())), numSamples * sizeof(ControlPoint));

                currentLayer[frame] = tmp;
            }

            _controlPoints.push_back(currentLayer);
            _segmentation.addLayer(TissueSegmentation::Layer());
        }

        size_t numVessels = 0;
        f.read(reinterpret_cast<char*>(&numVessels), sizeof(size_t));

        for (size_t i = 0; i < numVessels; ++i) {
            int frame = 0;
            f.read(reinterpret_cast<char*>(&frame), sizeof(int));

            tgt::vec4 centerline(-1.f);
            f.read(reinterpret_cast<char*>(&centerline), sizeof(tgt::vec4));

            _vesselBounds[frame] = centerline;
        }

        p_activeLayer.setMaxValue(_segmentation.getNumLayers()-1);
    }

    void ManualTissueSegmenter::onAddLayer() {
        _controlPoints.push_back(std::map< int, std::vector< ControlPoint > >());
        _segmentation.addLayer(TissueSegmentation::Layer());
        p_activeLayer.setMaxValue(_segmentation.getNumLayers()-1);
    }



}
