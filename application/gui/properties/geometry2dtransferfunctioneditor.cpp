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

#include "geometry2dtransferfunctioneditor.h"

#include "cgt/assert.h"
#include "cgt/shadermanager.h"
#include "cgt/glcontextmanager.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "application/gui/qtcolortools.h"
#include "application/gui/properties/tfgeometrymanipulator.h"

#include "core/classification/geometry2dtransferfunction.h"
#include "core/classification/tfgeometry2d.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/properties/transferfunctionproperty.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace campvis {

    Geometry2DTransferFunctionEditor::Geometry2DTransferFunctionEditor(TransferFunctionProperty* prop, Geometry2DTransferFunction* tf, QWidget* parent /*= 0*/)
        : AbstractTransferFunctionEditor(prop, tf, parent)
        , _layout(0)
        , _canvas(0)
        , _lblIntensityLeft(0)
        , _lblIntensityRight(0)
        , _btnAddGeometry(0)
    {
        _selectedGeometry = 0;
        setupGUI();
        tf->s_geometryCollectionChanged.connect(this, &Geometry2DTransferFunctionEditor::onGeometryCollectionChanged);
        updateManipulators();
        setEventTypes(cgt::Event::MOUSEPRESSEVENT);
    }

    Geometry2DTransferFunctionEditor::~Geometry2DTransferFunctionEditor() {
        disconnectFromTf();

        if (cgt::GlContextManager::isInited())
            cgt::GlContextManager::getRef().removeContext(_canvas);
    }

    void Geometry2DTransferFunctionEditor::updateWidgetFromProperty() {
        Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);
        _lblIntensityLeft->setText(QString::number(gtf->getIntensityDomain().x));
        _lblIntensityRight->setText(QString::number(gtf->getIntensityDomain().y));
        invalidate();
    }

    void Geometry2DTransferFunctionEditor::paint() {
        Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry2D*>& geometries = gtf->getGeometries();

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

        // renderIntoEditor TF geometries
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            for (std::vector<TFGeometry2D*>::const_iterator it = geometries.begin(); it != geometries.end(); ++it) {
                (*it)->render();
            }
        }

        // render histogram if existent
        const TransferFunctionProperty::IntensityHistogramType* ih = getIntensityHistogram();
        if (ih != 0) {
            size_t numBuckets = ih->getNumBuckets(0);
            if (numBuckets > 0) {
                float maxFilling = static_cast<float>(ih->getMaxFilling());

                float xl = static_cast<float>(0.f) / static_cast<float>(numBuckets);
                float yl = static_cast<float>(ih->getNumElements(0)) / maxFilling;

                glBegin(GL_QUADS);
                glColor4f(1.f, .75f, 0.f, .5f);
                for (size_t i = 1; i < numBuckets; ++i) {
                    float xr = static_cast<float>(i) / static_cast<float>(numBuckets);
                    float yr = static_cast<float>(ih->getNumElements(i)) / maxFilling;

                    glVertex2f(xl, 0.f);
                    glVertex2f(xl, yl);
                    glVertex2f(xr, yr);
                    glVertex2f(xr, 0.f);

                    xl = xr;
                    yl = yr;
                }
                glEnd();
            }
        }

        {
            tbb::mutex::scoped_lock lock(_localMutex);

            // render selected geometry
            if (_selectedGeometry != 0) {
                const std::vector<cgt::vec2>& helperPoints = _selectedGeometry->getHelperPoints();
                glColor4ub(0, 0, 0, 196);
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0xFAFA);
                glBegin(GL_LINE_LOOP);
                for (std::vector<cgt::vec2>::const_iterator it = helperPoints.begin(); it != helperPoints.end(); ++it)
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

    void Geometry2DTransferFunctionEditor::sizeChanged(const cgt::ivec2& size) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
                (*it)->setViewportSize(size);
            }
        }
        invalidate();
    }

    void Geometry2DTransferFunctionEditor::mousePressEvent(cgt::MouseEvent* e) {
        if (_selectedGeometry != 0 && e->modifiers() & cgt::Event::CTRL) {
//             TFGeometry2D* g = _selectedGeometry->getGeometry();
//             std::vector<TFGeometry2D::KeyPoint>& kpts = g->getKeyPoints();
//             TFGeometry2D::KeyPoint kp(static_cast<float>(e->x()) / static_cast<float>(_canvas->width()), cgt::col4(255));
//             std::vector<TFGeometry2D::KeyPoint>::const_iterator lb = std::upper_bound(kpts.begin(), kpts.end(), kp);
//             if (lb != kpts.end()) {
//                 kp._color = lb->_color;
//             }
//             else {
//                 kp._color = kpts.back()._color;
//             }
//             float alpha = cgt::clamp(static_cast<float>(_canvas->height() - e->y()) / static_cast<float>(_canvas->height()), 0.f, 1.f);
//             kp._color.a = static_cast<uint8_t>(alpha * 255.f);
//             kpts.insert(lb, kp);
//             updateManipulators();
//             g->s_changed();
        }
        else {
            _selectedGeometry = 0;
            invalidate();
            e->ignore();
        }
    }

    void Geometry2DTransferFunctionEditor::repaint() {
        invalidate();
    }

    void Geometry2DTransferFunctionEditor::invalidate() {
        // TODO: check, whether this should be done in an extra thread
        cgt::GLContextScopedLock lock(_canvas);
        paint();
    }

    void Geometry2DTransferFunctionEditor::setupGUI() {
        Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);

        _layout = new QGridLayout(this);
        setLayout(_layout);

        QLabel* lblOpacityTop = new QLabel(tr("100%"), this);
        _layout->addWidget(lblOpacityTop, 1, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacity = new QLabel(tr("Opacity"), this);
        _layout->addWidget(lblOpacity, 2, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacityBottom = new QLabel(tr("0%"), this);
        _layout->addWidget(lblOpacityBottom, 3, 0, 1, 1, Qt::AlignRight);

        _canvas = new cgt::QtThreadedCanvas("", cgt::ivec2(256, 128), cgt::GLCanvas::RGBA_BUFFER, 0, false);
        GLCtxtMgr.registerContextAndInitGlew(_canvas, "Geometry2DTransferFunctionEditor");
        GLCtxtMgr.releaseContext(_canvas, false);

        _canvas->setPainter(this);
        _layout->addWidget(_canvas, 1, 1, 3, 3);

        _lblIntensityLeft = new QLabel(QString::number(gtf->getIntensityDomain().x), this);
        _layout->addWidget(_lblIntensityLeft, 4, 1, 1, 1, Qt::AlignLeft);
        QLabel* lblIntensity = new QLabel(tr("Intensity"), this);
        _layout->addWidget(lblIntensity, 4, 2, 1, 1, Qt::AlignHCenter);
        _lblIntensityRight = new QLabel(QString::number(gtf->getIntensityDomain().y), this);
        _layout->addWidget(_lblIntensityRight, 4, 3, 1, 1, Qt::AlignRight);

        QVBoxLayout* buttonLayout = new QVBoxLayout();
        _layout->addLayout(buttonLayout, 1, 4, 1, 3, Qt::AlignTop);

        _btnAddGeometry = new QPushButton(tr("Add Geometry"), this);
        buttonLayout->addWidget(_btnAddGeometry);
        connect(_btnAddGeometry, SIGNAL(clicked()), this, SLOT(onBtnAddGeometryClicked()));
        _btnRemoveGeometry = new QPushButton(tr("Remove Geometry"), this);
        buttonLayout->addWidget(_btnRemoveGeometry);
        connect(_btnRemoveGeometry, SIGNAL(clicked()), this, SLOT(onBtnRemoveGeometryClicked()));

        _layout->setColumnStretch(2, 1);
        _layout->setRowStretch(2, 1);
    }

    void Geometry2DTransferFunctionEditor::updateManipulators() {
        tbb::mutex::scoped_lock lock(_localMutex);

        _canvas->getEventHandler()->addEventListenerToFront(this);
    }

    void Geometry2DTransferFunctionEditor::onGeometryCollectionChanged() {
        updateManipulators();
    }

    void Geometry2DTransferFunctionEditor::onWholeTFGeometryManipulatorSelected(WholeTFGeometryManipulator* wtf /* :) */) {
        _selectedGeometry = wtf;
        invalidate();
    }

    void Geometry2DTransferFunctionEditor::onBtnAddGeometryClicked() {
        Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);
        gtf->addGeometry(TFGeometry2D::createQuad(cgt::vec2(.4f, .6f), cgt::vec2(0.f, .4f), cgt::col4(196)));
    }

    void Geometry2DTransferFunctionEditor::onBtnRemoveGeometryClicked() {
        if (_selectedGeometry != 0) {
            // to get the signal-slots disconnected in the correct order and avoid double deletion,
            // this is getting a little messy and cumbersome:
//             Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);
//             TFGeometry2D* geometryToRemove = _selectedGeometry->getGeometry();
// 
//             {
//                 tbb::mutex::scoped_lock lock(_localMutex);
// 
//                 for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
//                     if (*it == _selectedGeometry) {
//                         _manipulators.erase(it);
//                         break;
//                     }
//                 }
//                 delete _selectedGeometry;
//                 _selectedGeometry = 0;
//             }
// 
//             gtf->removeGeometry(geometryToRemove);
        }
    }

    void Geometry2DTransferFunctionEditor::onTfAboutToBeDeleted() {
        disconnectFromTf();
    }

    void Geometry2DTransferFunctionEditor::disconnectFromTf() {
        tbb::mutex::scoped_lock lock(_localMutex);

        // clear and delete former stuff
        _selectedGeometry = 0;
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            if (WholeTFGeometryManipulator* tester = dynamic_cast<WholeTFGeometryManipulator*>(*it)) {
                tester->s_selected.disconnect(this);
            }
            delete *it;
        }
        _manipulators.clear();

        Geometry2DTransferFunction* gtf = static_cast<Geometry2DTransferFunction*>(_transferFunction);
        if (gtf != nullptr) {
            gtf->s_geometryCollectionChanged.disconnect(this);
            gtf->s_aboutToBeDeleted.disconnect(this);
            _transferFunction->s_changed.disconnect(this);
            _transferFunction = nullptr;
        }
    }


}
