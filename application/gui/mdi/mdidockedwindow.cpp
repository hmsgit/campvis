// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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
