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

#include "mdidockedwindow.h"

#include <QMdiArea>
#include <QMoveEvent>

namespace campvis {

    MdiDockedWindow::MdiDockedWindow(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
        : QMdiSubWindow(parent, flags)
        , _dragActive(false)
        , _lastMousePos()
    {}

    void MdiDockedWindow::stopWindowDrag() {
        if (_dragActive) {
            _dragActive = false;
            releaseMouse();
        }
    }

    void MdiDockedWindow::mouseMoveEvent(QMouseEvent* event) {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            const QPoint& mousePos = event->globalPos();

            if (!_dragActive) {
                _dragActive = true;
                _lastMousePos = mousePos;
                return QMdiSubWindow::mouseMoveEvent(event);
            }

            QPoint newPos = pos() + (mousePos - _lastMousePos);

            /*
             * Dragging the subwindow upwards out of the MDI area is blocked for 2 reasons:
             * - the subwindow can't be detached and focused reliably in such cases, possibly due
             *   to the main window's title bar being in the way
             * - that's how moving subwindows in an MDI area works by default
             */
            if (newPos.y() < 0) {
                newPos.setY(0);
                _lastMousePos.setX(mousePos.x());
            }
            else {
                _lastMousePos = mousePos;
            }

            move(newPos);
            emit s_positionChanged(this, newPos);
        }
        else {
            QMdiSubWindow::mouseMoveEvent(event);
        }
    }

    void MdiDockedWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton) {
            stopWindowDrag();
            mdiArea()->tileSubWindows();
        }

        // The default implementation detects clicks on the close, maximize and minimize buttons,
        // among other things
        QMdiSubWindow::mouseReleaseEvent(event);
    }

}
