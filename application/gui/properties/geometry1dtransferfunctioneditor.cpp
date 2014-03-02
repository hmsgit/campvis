// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "geometry1dtransferfunctioneditor.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/glcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#include "application/gui/qtcolortools.h"
#include "application/gui/properties/tfgeometrymanipulator.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/tools/opengljobprocessor.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace campvis {

    Geometry1DTransferFunctionEditor::Geometry1DTransferFunctionEditor(TransferFunctionProperty* prop, Geometry1DTransferFunction* tf, QWidget* parent /*= 0*/)
        : AbstractTransferFunctionEditor(prop, tf, parent)
        , _logScale(true)
        , _layout(0)
        , _canvas(0)
        , _lblIntensityLeft(0)
        , _lblIntensityRight(0)
        , _btnAddGeometry(0)
        , _btnRemoveGeometry(0)
        , _cbLogScale(0)
    {
        _selectedGeometry = 0;
        setupGUI();
        tf->s_geometryCollectionChanged.connect(this, &Geometry1DTransferFunctionEditor::onGeometryCollectionChanged);
        updateManipulators();
        setEventTypes(tgt::Event::MOUSEPRESSEVENT);
    }

    Geometry1DTransferFunctionEditor::~Geometry1DTransferFunctionEditor() {
        tbb::mutex::scoped_lock lock(_localMutex);

        // clearEventListeners and delete former stuff
        _selectedGeometry = 0;
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            if (WholeTFGeometryManipulator* tester = dynamic_cast<WholeTFGeometryManipulator*>(*it)) {
                tester->s_selected.disconnect(this);
            }
            delete *it;
        }

        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
        gtf->s_geometryCollectionChanged.disconnect(this);

        if (OpenGLJobProcessor::isInited())
            GLJobProc.deregisterContext(_canvas);
        if (tgt::GlContextManager::isInited())
            tgt::GlContextManager::getRef().removeContext(_canvas);
    }

    void Geometry1DTransferFunctionEditor::updateWidgetFromProperty() {
        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
        _lblIntensityLeft->setText(QString::number(gtf->getIntensityDomain().x));
        _lblIntensityRight->setText(QString::number(gtf->getIntensityDomain().y));
        invalidate();
    }

    void Geometry1DTransferFunctionEditor::paint() {
        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry1D*>& geometries = gtf->getGeometries();

        // TODO: get rid of intermediate mode?
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, _canvas->width(), _canvas->height());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(0, 1 , 0, 1, -1, 1);
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        LGL_ERROR;
        
        // render histogram if existent
        const TransferFunctionProperty::IntensityHistogramType* ih = getIntensityHistogram();
        if (ih != 0) {
            size_t numBuckets = ih->getNumBuckets(0);
            if (numBuckets > 0) {
                float maxFilling = static_cast<float>(ih->getMaxFilling());
                if (_logScale)
                    maxFilling = log(maxFilling);

                float xl = static_cast<float>(0.f) / static_cast<float>(numBuckets);
                float yl = (_logScale 
                    ? log(static_cast<float>(ih->getNumElements(0))) / maxFilling
                    : static_cast<float>(ih->getNumElements(0)) / maxFilling);


                glBegin(GL_QUADS);
                glColor4f(1.f, .75f, 0.f, .5f);
                for (size_t i = 1; i < numBuckets; ++i) {
                    float xr = static_cast<float>(i) / static_cast<float>(numBuckets);
                    float yr = (_logScale 
                        ? std::max(0.f, static_cast<float>(log(static_cast<float>(ih->getNumElements(i)))) / maxFilling)
                        : static_cast<float>(ih->getNumElements(i)) / maxFilling);
                    
                    glVertex2f(xl, 0.f);
                    glVertex2f(xl, yl);
                    glVertex2f(xr, yr);
                    glVertex2f(xr, 0.f);
                }
                glEnd();
            }
        }

        // render TF geometries
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            for (std::vector<TFGeometry1D*>::const_iterator it = geometries.begin(); it != geometries.end(); ++it) {
                (*it)->renderIntoEditor();
            }
        }

        {
            tbb::mutex::scoped_lock lock(_localMutex);

            // render selected geometry
            WholeTFGeometryManipulator* selectedGeometry = _selectedGeometry;
            if (selectedGeometry != 0) {
                // the copy is deliberate for improved thread safety (the whole design is a little messy here...)
                std::vector<tgt::vec2> helperPoints = selectedGeometry->getHelperPoints();
                glColor4ub(0, 0, 0, 196);
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0xFAFA);
                glBegin(GL_LINE_LOOP);
                for (std::vector<tgt::vec2>::const_iterator it = helperPoints.begin(); it != helperPoints.end(); ++it)
                    glVertex2fv(it->elem);
                glEnd();
                glDisable(GL_LINE_STIPPLE);
            }

            glPopMatrix();

            glPushMatrix();
            glOrtho(0, _canvas->width(), 0, _canvas->height(), -1, 1);
            // render manipulators
            for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
                (*it)->render();
            }
            glPopMatrix();
        }

        LGL_ERROR;
        glPopAttrib();
    }

    void Geometry1DTransferFunctionEditor::sizeChanged(const tgt::ivec2& size) {
        tbb::mutex::scoped_lock lock(_localMutex);
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            (*it)->setViewportSize(size);
        }
        invalidate();
    }

    void Geometry1DTransferFunctionEditor::mousePressEvent(tgt::MouseEvent* e) {
        if (_selectedGeometry != 0 && e->modifiers() & tgt::Event::CTRL) {
            TFGeometry1D* g = _selectedGeometry->getGeometry();
            {
                tbb::mutex::scoped_lock lock(_localMutex);

                // add a control point on CTRL+Click
                std::vector<TFGeometry1D::KeyPoint>& kpts = g->getKeyPoints();
                TFGeometry1D::KeyPoint kp(static_cast<float>(e->x()) / static_cast<float>(_canvas->width()), tgt::col4(255));
                std::vector<TFGeometry1D::KeyPoint>::iterator lb = std::upper_bound(kpts.begin(), kpts.end(), kp);
                if (lb != kpts.end()) {
                    kp._color = lb->_color;
                }
                else {
                    kp._color = kpts.back()._color;
                }
                float alpha = tgt::clamp(static_cast<float>(_canvas->height() - e->y()) / static_cast<float>(_canvas->height()), 0.f, 1.f);
                kp._color.a = static_cast<uint8_t>(alpha * 255.f);
                kpts.insert(lb, kp);
            }

            updateManipulators();
            g->s_changed();
        }
        else {
            _selectedGeometry = 0;
            invalidate();
            e->ignore();
        }
    }

    void Geometry1DTransferFunctionEditor::repaint() {
        invalidate();
    }

    void Geometry1DTransferFunctionEditor::invalidate() {
        GLJobProc.enqueueJob(_canvas, makeJobOnHeap(this, &Geometry1DTransferFunctionEditor::paint), OpenGLJobProcessor::PaintJob);
    }

    void Geometry1DTransferFunctionEditor::setupGUI() {
        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);

        _layout = new QGridLayout(this);
        setLayout(_layout);

        QLabel* lblOpacityTop = new QLabel(tr("100%"), this);
        _layout->addWidget(lblOpacityTop, 1, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacity = new QLabel(tr("Opacity"), this);
        _layout->addWidget(lblOpacity, 2, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacityBottom = new QLabel(tr("0%"), this);
        _layout->addWidget(lblOpacityBottom, 3, 0, 1, 1, Qt::AlignRight);

        _canvas = new tgt::QtThreadedCanvas("", tgt::ivec2(256, 128), tgt::GLCanvas::RGBA_BUFFER, 0, false);
        tgt::GlContextManager::getRef().registerContextAndInitGlew(_canvas);

        GLJobProc.registerContext(_canvas);
        _canvas->setPainter(this, false);
        _layout->addWidget(_canvas, 1, 1, 3, 3);

        _lblIntensityLeft = new QLabel(QString::number(gtf->getIntensityDomain().x), this);
        _layout->addWidget(_lblIntensityLeft, 4, 1, 1, 1, Qt::AlignLeft);
        QLabel* lblIntensity = new QLabel(tr("Intensity"), this);
        _layout->addWidget(lblIntensity, 4, 2, 1, 1, Qt::AlignHCenter);
        _lblIntensityRight = new QLabel(QString::number(gtf->getIntensityDomain().y), this);
        _layout->addWidget(_lblIntensityRight, 4, 3, 1, 1, Qt::AlignRight);

        QVBoxLayout* buttonLayout = new QVBoxLayout(); // TODO: check whether buttonLayout will be deleted by Qt's GC!
        _layout->addLayout(buttonLayout, 1, 4, 1, 3, Qt::AlignTop);

        _btnAddGeometry = new QPushButton(tr("Add Geometry"), this);
        buttonLayout->addWidget(_btnAddGeometry);
        connect(_btnAddGeometry, SIGNAL(clicked()), this, SLOT(onBtnAddGeometryClicked()));
        _btnRemoveGeometry = new QPushButton(tr("Remove Geometry"), this);
        buttonLayout->addWidget(_btnRemoveGeometry);
        connect(_btnRemoveGeometry, SIGNAL(clicked()), this, SLOT(onBtnRemoveGeometryClicked()));
        _cbLogScale = new QCheckBox(tr("Logarithmic Scale"), this);
        _cbLogScale->setChecked(true);
        buttonLayout->addWidget(_cbLogScale);
        connect(_cbLogScale, SIGNAL(stateChanged(int)), this, SLOT(onCbLogScaleStateChanged(int)));
        _layout->setColumnStretch(2, 1);
        _layout->setRowStretch(2, 1);
    }

    void Geometry1DTransferFunctionEditor::updateManipulators() {
        tbb::mutex::scoped_lock lock(_localMutex);

        // clearEventListeners and delete former stuff
        _selectedGeometry = 0;
        _canvas->getEventHandler()->clearEventListeners();
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            if (WholeTFGeometryManipulator* tester = dynamic_cast<WholeTFGeometryManipulator*>(*it)) {
            	tester->s_selected.disconnect(this);
            }
            delete *it;
        }
        _manipulators.clear();

        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry1D*>& geometries = gtf->getGeometries();
        for (std::vector<TFGeometry1D*>::const_iterator git = geometries.begin(); git != geometries.end(); ++git) {
            // Add manipulator for the whole geometry and register it as event handler:
            WholeTFGeometryManipulator* wtf = new WholeTFGeometryManipulator(_canvas->getSize(), *git);
            _manipulators.push_back(wtf);
            _canvas->getEventHandler()->addEventListenerToFront(wtf);
            wtf->s_selected.connect(this, &Geometry1DTransferFunctionEditor::onWholeTFGeometryManipulatorSelected);

            // Add a manipulator for each KeyPoint and register it as event handler:
            for (std::vector<TFGeometry1D::KeyPoint>::iterator kpit = (*git)->getKeyPoints().begin(); kpit != (*git)->getKeyPoints().end(); ++kpit) {
                _manipulators.push_back(new KeyPointManipulator(_canvas->getSize(), *git, kpit));
                _canvas->getEventHandler()->addEventListenerToFront(_manipulators.back());
            }
        }

        _canvas->getEventHandler()->addEventListenerToFront(this);
    }

    void Geometry1DTransferFunctionEditor::onGeometryCollectionChanged() {
        updateManipulators();
    }

    void Geometry1DTransferFunctionEditor::onWholeTFGeometryManipulatorSelected(WholeTFGeometryManipulator* wtf /* :) */) {
        _selectedGeometry = wtf;
        invalidate();
    }

    void Geometry1DTransferFunctionEditor::onBtnAddGeometryClicked() {
        Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
        gtf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .6f), tgt::col4(196), tgt::col4(196)));
    }

    void Geometry1DTransferFunctionEditor::onBtnRemoveGeometryClicked() {
        if (_selectedGeometry != 0) {
            // to get the signal-slots disconnected in the correct order and avoid double deletion,
            // this is getting a little messy and cumbersome:
            Geometry1DTransferFunction* gtf = static_cast<Geometry1DTransferFunction*>(_transferFunction);
            TFGeometry1D* geometryToRemove = _selectedGeometry->getGeometry();

            {
                tbb::mutex::scoped_lock lock(_localMutex);

                for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
                    if (*it == _selectedGeometry) {
                        _manipulators.erase(it);
                        break;
                    }
                }
                delete _selectedGeometry;
                _selectedGeometry = 0;
            }

            gtf->removeGeometry(geometryToRemove);
        }
    }

    void Geometry1DTransferFunctionEditor::onCbLogScaleStateChanged(int state) {
        _logScale = (state & Qt::Checked);
        invalidate();
    }


}
