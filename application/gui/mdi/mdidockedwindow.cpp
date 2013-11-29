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
#include <QStyle>

namespace campvis {

    MdiDockedWindow::MdiDockedWindow(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
        : QMdiSubWindow(parent)
        , _dragActive(false)
        , _lastMousePos()
    {
        this->setWindowFlags(flags | Qt::Tool);
    }

    void MdiDockedWindow::forceWindowDrag() {
        _dragActive = true;
        _lastMousePos = QCursor::pos();

        // Position the window so that the centre of its title bar is under the cursor
        const QPoint& mousePos = this->mapToParent(this->mapFromGlobal(_lastMousePos));
        int x = mousePos.x() - this->frameSize().width() / 2;
        int y = mousePos.y() - this->style()->pixelMetric(QStyle::PM_TitleBarHeight) / 2;
        QPoint newPos = QPoint(x, y);

        move(newPos);
        this->grabMouse();
    }

    void MdiDockedWindow::stopWindowDrag() {
        if (_dragActive) {
            _dragActive = false;
            this->releaseMouse();
        }
    }

    void MdiDockedWindow::mouseMoveEvent(QMouseEvent* event) {
        /*
         * Only intercept mouse move events if the window is being dragged and the left mouse
         * button is pressed.
         */
        if (_dragActive && event->buttons().testFlag(Qt::LeftButton)) {
            const QPoint& mousePos = event->globalPos();
            QPoint newPos = pos() + (mousePos - _lastMousePos);

            /*
             * Dragging the subwindow upwards out of the MDI area is blocked for 2 reasons:
             * - the docked window can't be detached and focused reliably in such cases, possibly
             *   due to the main window's title bar being in the way
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
            emit s_positionChanged(newPos);
        }
        else
            QMdiSubWindow::mouseMoveEvent(event);
    }

    void MdiDockedWindow::mousePressEvent(QMouseEvent* event) {
        const QPoint& widgetPos = this->widget()->mapFromParent(event->pos());

        /*
         * Mouse drag detection starts only in response to non-resize (the window's current cursor
         * is the default one) drag (the left mouse button is pressed) events; additionally, the
         * mouse pointer has to be on the title bar.
         */
        if (event->button() == Qt::LeftButton && widgetPos.y() < 0 && this->cursor().shape() == Qt::ArrowCursor) {
            _dragActive = true;
            _lastMousePos = event->globalPos();
        }

        QMdiSubWindow::mousePressEvent(event);
    }

    void MdiDockedWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton)
            stopWindowDrag();

        // The default implementation detects clicks on the close, maximize and minimize buttons,
        // among other things
        QMdiSubWindow::mouseReleaseEvent(event);
    }

    void MdiDockedWindow::closeEvent(QCloseEvent* /*event*/) {
        emit s_closed();
    }

}
