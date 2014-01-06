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

#include "mdifloatingwindow.h"

#include <QMdiArea>
#include <QMoveEvent>
#include <QHBoxLayout>
#include <QStyle>

namespace campvis {

    MdiFloatingWindow::MdiFloatingWindow(QWidget* widget, QWidget* parent /*= 0*/)
        : QWidget(parent, Qt::Tool)
        , _widget(widget)
        , _dragActive(false)
    {
        QLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(widget);

        setLayout(layout);
    }

    void MdiFloatingWindow::forceWindowDrag() {
        if (!_dragActive) {
            _dragActive = true;
            this->snapToCursor(QCursor::pos());
            grabMouse();
        }
    }

    void MdiFloatingWindow::stopWindowDrag() {
        if (_dragActive) {
            _dragActive = false;
            releaseMouse();
        }
    }

    QWidget* MdiFloatingWindow::widget() const {
        return _widget;
    }

    void MdiFloatingWindow::mouseMoveEvent(QMouseEvent* event) {
        if (_dragActive)
            this->snapToCursor(event->globalPos());
        else
            QWidget::mouseMoveEvent(event);
    }

    void MdiFloatingWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::LeftButton) {
            stopWindowDrag();
        }
    }

    void MdiFloatingWindow::moveEvent(QMoveEvent* /*event*/) {
        emit s_positionChanged(frameGeometry().topLeft());
    }

    void MdiFloatingWindow::snapToCursor(const QPoint& cursorPos) {
        int x = cursorPos.x() - this->frameSize().width() / 2;
        int y = cursorPos.y() - this->style()->pixelMetric(QStyle::PM_TitleBarHeight) / 2;

        this->move(x, y);
    }

    void MdiFloatingWindow::closeEvent(QCloseEvent* /*event*/) {
        emit s_closed();
    }

}