// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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
#include "core/classification/geometrytransferfunction.h"
#include "core/classification/tfgeometry.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/tools/opengljobprocessor.h"

#include <QGridLayout>
#include <QLabel>

namespace TUMVis {

    GeometryTransferFunctionEditor::GeometryTransferFunctionEditor(GeometryTransferFunction* tf, QWidget* parent /*= 0*/)
        : AbstractTransferFunctionEditor(tf, parent)
        , _layout(0)
        , _canvas(0)
        , _shader(0)
    {
        _layout = new QGridLayout(this);
        setLayout(_layout);

        _canvas = CtxtMgr.createContext("tfcanvas", "", tgt::ivec2(256, 128), tgt::GLCanvas::RGBA_BUFFER, this, false);
        _canvas->doneCurrent();
        GLJobProc.registerContext(_canvas);
        _canvas->getEventHandler()->addListenerToBack(this);
        _canvas->setPainter(this, false);
        _layout->addWidget(_canvas, 0, 0);

        GLJobProc.enqueueJob(_canvas, new CallMemberFuncJob<GeometryTransferFunctionEditor>(this, &GeometryTransferFunctionEditor::init), OpenGLJobProcessor::SerialJob);
    }

    GeometryTransferFunctionEditor::~GeometryTransferFunctionEditor() {
        //GLJobProc.deregisterContext(_canvas);
    }

    void GeometryTransferFunctionEditor::updateWidgetFromProperty() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
    }

    void GeometryTransferFunctionEditor::init() {
        tgtAssert(_shader == 0, "GeometryTransferFunctionEditor already inited.");
    }

    void GeometryTransferFunctionEditor::paint() {
        GeometryTransferFunction* gtf = static_cast<GeometryTransferFunction*>(_transferFunction);
        const std::vector<TFGeometry*>& geometries = gtf->getGeometries();
        const tgt::vec2& intensityDomain = gtf->getIntensityDomain();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        glViewport(0, 0, _canvas->width(), _canvas->height());
        glOrtho(0, 1, 0, 1, -1, 1);
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        LGL_ERROR;

//         glBegin(GL_QUADS);
//             glColor3f(1.f, 0.f, 0.f);
//             glVertex2f(0.f, 0.f);
//             glColor3f(1.f, 1.f, 0.f);
//             glVertex2f(1.f, 0.f);
//             glColor3f(0.f, 1.f, 0.f);
//             glVertex2f(1.f, 1.f);
//             glColor3f(0.f, 0.f, 1.f);
//             glVertex2f(0.f, 1.f);
//         glEnd();

        for (std::vector<TFGeometry*>::const_iterator it = geometries.begin(); it != geometries.end(); ++it) {
            (*it)->render();
        }

        LGL_ERROR;
        glPopMatrix();
        glPopAttrib();
    }

    void GeometryTransferFunctionEditor::sizeChanged(const tgt::ivec2&) {
        invalidate();
    }

    void GeometryTransferFunctionEditor::invalidate() {
        GLJobProc.enqueueJob(_canvas, new CallMemberFuncJob<GeometryTransferFunctionEditor>(this, &GeometryTransferFunctionEditor::paint), OpenGLJobProcessor::PaintJob);
    }


}