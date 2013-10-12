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

#include "mdifloatingwindow.h"

#include <QMdiArea>
#include <QMoveEvent>
#include <QHBoxLayout>

namespace campvis {

    MdiFloatingWindow::MdiFloatingWindow(QWidget* widget, QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _widget(widget)
        , _dragActive(false)
        , _lastMousePos()
    {
        QLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(widget);

        setLayout(layout);
    }

    void MdiFloatingWindow::forceWindowDrag() {
        if (!_dragActive && parent() == 0) {
            _dragActive = true;
            _lastMousePos = QCursor::pos();
            grabMouse();
        }
    }

    void MdiFloatingWindow::stopWindowDrag() {
        if (_dragActive) {
            _dragActive = false;
            releaseMouse();
        }
    }

    QWidget* MdiFloatingWindow::widget() {
        return _widget;
    }

    void MdiFloatingWindow::mouseMoveEvent(QMouseEvent* event) {
        const QPoint& mousePos = event->globalPos();
        const QPoint& newPos = pos() + (mousePos - _lastMousePos);

        move(newPos);
        _lastMousePos = mousePos;
    }

    void MdiFloatingWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton) {
            stopWindowDrag();
        }
    }

    void MdiFloatingWindow::moveEvent(QMoveEvent* /*event*/) {
        emit s_positionChanged(this, frameGeometry().topLeft());
    }

}
