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

#include "geometrytransferfunctioneditor.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#include "application/gui/qtcolortools.h"
#include "application/gui/properties/tfgeometrymanipulator.h"

#include "core/classification/geometrytransferfunction.h"
#include "core/classification/tfgeometry.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/tools/opengljobprocessor.h"

#include <QGridLayout>
#include <QLabel>

namespace TUMVis {

    GeometryTransferFunctionEditor::GeometryTransferFunctionEditor(GeometryTransferFunction* tf, QWidget* parent /*= 0*/)
        : AbstractTransferFunctionEditor(tf, parent)
        , _layout(0)
        , _canvas(0)
        , _lblIntensityLeft(0)
        , _lblIntensityRight(0)
    {
        setupGUI();
        tf->s_geometryCollectionChanged.connect(this, &GeometryTransferFunctionEditor::onGeometryCollectionChanged);
        updateManipulators();
    }

    GeometryTransferFunctionEditor::~GeometryTransferFunctionEditor() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
        gtf->s_geometryCollectionChanged.disconnect(this);
        // TODO: this needs to be done, but we can not ensure that GLJobProc is still existant during deconstruction...
        //GLJobProc.deregisterContext(_canvas);
    }

    void GeometryTransferFunctionEditor::updateWidgetFromProperty() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
        _lblIntensityLeft->setText(QString::number(gtf->getIntensityDomain().x));
        _lblIntensityRight->setText(QString::number(gtf->getIntensityDomain().y));
        invalidate();
    }

    void GeometryTransferFunctionEditor::paint() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry*>& geometries = gtf->getGeometries();
        const tgt::vec2& intensityDomain = gtf->getIntensityDomain();

        // TODO: get rid of intermediate mode?
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glViewport(0, 0, _canvas->width(), _canvas->height());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(0, 1 , 0, 1, -1, 1);
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        LGL_ERROR;

        // render TF geometries
        for (std::vector<TFGeometry*>::const_iterator it = geometries.begin(); it != geometries.end(); ++it) {
            (*it)->render();
        }

        // render histogram if existent
        const AbstractTransferFunction::IntensityHistogramType* ih = gtf->getIntensityHistogram();
        if (ih != 0) {
            size_t numBuckets = ih->getNumBuckets(0);
            if (numBuckets > 0) {
                float maxFilling = static_cast<float>(ih->getMaxFilling());

                float xl = static_cast<float>(0.f) / static_cast<float>(numBuckets);
                float xr = 0.f;
                float yl = static_cast<float>(ih->getNumElements(0)) / maxFilling;
                float yr = 0.f;

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glColor4f(1.f, .75f, 0.f, .5f);
                for (size_t i = 1; i < numBuckets; ++i) {
                    xr = static_cast<float>(i) / static_cast<float>(numBuckets);
                    yr = static_cast<float>(ih->getNumElements(i)) / maxFilling;

                    glVertex2f(xl, 0.f);
                    glVertex2f(xl, yl);
                    glVertex2f(xr, yr);
                    glVertex2f(xr, 0.f);

                    xl = xr;
                    yl = yr;
                }
                glEnd();
                glDisable(GL_BLEND);
            }
        }
        glPopMatrix();

        glPushMatrix();
        glOrtho(0, _canvas->width(), 0, _canvas->height(), -1, 1);
        // render manipulators
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            (*it)->render();
        }
        glPopMatrix();

        LGL_ERROR;
        glPopAttrib();
    }

    void GeometryTransferFunctionEditor::sizeChanged(const tgt::ivec2& size) {
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            (*it)->setViewportSize(size);
        }
        invalidate();
    }

    void GeometryTransferFunctionEditor::invalidate() {
        GLJobProc.enqueueJob(_canvas, new CallMemberFuncJob<GeometryTransferFunctionEditor>(this, &GeometryTransferFunctionEditor::paint), OpenGLJobProcessor::PaintJob);
    }

    void GeometryTransferFunctionEditor::setupGUI() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);

        _layout = new QGridLayout(this);
        setLayout(_layout);

        QLabel* lblOpacityTop = new QLabel(tr("100%"), this);
        _layout->addWidget(lblOpacityTop, 1, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacity = new QLabel(tr("Opacity"), this);
        _layout->addWidget(lblOpacity, 2, 0, 1, 1, Qt::AlignRight);
        QLabel* lblOpacityBottom = new QLabel(tr("0%"), this);
        _layout->addWidget(lblOpacityBottom, 3, 0, 1, 1, Qt::AlignRight);

        _canvas = CtxtMgr.createContext("tfcanvas", "", tgt::ivec2(256, 128), tgt::GLCanvas::RGBA_BUFFER, 0, false);
        _canvas->doneCurrent();
        GLJobProc.registerContext(_canvas);
        _canvas->setPainter(this, false);
        _layout->addWidget(_canvas, 1, 1, 3, 3);

        _lblIntensityLeft = new QLabel(QString::number(gtf->getIntensityDomain().x), this);
        _layout->addWidget(_lblIntensityLeft, 4, 1, 1, 1, Qt::AlignLeft);
        QLabel* lblIntensity = new QLabel(tr("Intensity"), this);
        _layout->addWidget(lblIntensity, 4, 2, 1, 1, Qt::AlignHCenter);
        _lblIntensityRight = new QLabel(QString::number(gtf->getIntensityDomain().y), this);
        _layout->addWidget(_lblIntensityRight, 4, 3, 1, 1, Qt::AlignRight);

        _layout->setColumnStretch(2, 1);
        _layout->setRowStretch(2, 1);
    }

    void GeometryTransferFunctionEditor::updateManipulators() {
        // clear and delete former stuff
        _canvas->getEventHandler()->clear();
        for (std::vector<AbstractTFGeometryManipulator*>::iterator it = _manipulators.begin(); it != _manipulators.end(); ++it) {
            delete *it;
        }
        _manipulators.clear();

        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry*>& geometries = gtf->getGeometries();
        for (std::vector<TFGeometry*>::const_iterator git = geometries.begin(); git != geometries.end(); ++git) {
            // Add manipulator for the whole geometry and register it as event handler:
            _manipulators.push_back(new WholeTFGeometryManipulator(_canvas->getSize(), gtf, *git));
            _canvas->getEventHandler()->addListenerToBack(_manipulators.back());

            // Add a manipulator for each KeyPoint and register it as event handler:
            for (std::vector<TFGeometry::KeyPoint>::iterator kpit = (*git)->getKeyPoints().begin(); kpit != (*git)->getKeyPoints().end(); ++kpit) {
                _manipulators.push_back(new KeyPointManipulator(_canvas->getSize(), gtf, *git, kpit));
                _canvas->getEventHandler()->addListenerToBack(_manipulators.back());
            }
        }

    }

    void GeometryTransferFunctionEditor::onGeometryCollectionChanged() {
        updateManipulators();
    }


}