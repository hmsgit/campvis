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

#include "mdidockablewindow.h"
#include "mdidockarea.h"

#include <QAction>

namespace campvis {

    MdiDockableWindow::MdiDockableWindow(QWidget* widget, MdiDockArea* mdiArea, Qt::WindowFlags windowFlags /*= 0*/)
        : QWidget(mdiArea)
        , _docked(true)
        , _mdiArea(mdiArea)
        , _dockedWindow(0)
        , _floatingWindow(0)
        , _toggleViewAction(0)
    {
        this->setWindowFlags(windowFlags);
        _dockedWindow = this->newDockedWindow(widget);

        _toggleViewAction = new QAction(this);
        _toggleViewAction->setCheckable(true);
        _toggleViewAction->setChecked(false);
        this->connect(_toggleViewAction, SIGNAL(toggled(bool)), SLOT(toggleWindowVisibility(bool)));
    }

    void MdiDockableWindow::setWindowTitle(const QString& title) {
        QWidget::setWindowTitle(title);
        _toggleViewAction->setText(title);

        if (_docked)
            _dockedWindow->setWindowTitle(title);
        else
            _floatingWindow->setWindowTitle(title);
    }

    void MdiDockableWindow::activateWindow() {
        if (_docked)
            _dockedWindow->setFocus();
        else
            _floatingWindow->activateWindow();
    }

    QAction* MdiDockableWindow::toggleViewAction() const {
        return _toggleViewAction;
    }

    void MdiDockableWindow::setVisible(bool visible) {
        _toggleViewAction->setChecked(visible);
    }

    MdiDockedWindow* MdiDockableWindow::newDockedWindow(QWidget* widget) {
        MdiDockedWindow* dockedWindow = new MdiDockedWindow(_mdiArea, this->windowFlags());
        dockedWindow->setWidget(widget);

        this->connect(dockedWindow, SIGNAL(s_positionChanged(const QPoint&)), SLOT(trackDockedWindowPosition(QPoint)));
        this->connect(dockedWindow, SIGNAL(s_closed()), SLOT(handleWindowClosing()));

        return dockedWindow;
    }

    void MdiDockableWindow::toggleWindowVisibility(bool on) {
        if (on) {
            if (_docked)
                _mdiArea->addSubWindow(_dockedWindow);
            else
                _floatingWindow->show();
        }
        else {
            if (_docked)
                _mdiArea->removeSubWindow(_dockedWindow);
            else
                _floatingWindow->hide();
        }
    }

    void MdiDockableWindow::handleWindowClosing() {
        _toggleViewAction->setChecked(false);
    }

    void MdiDockableWindow::trackFloatingWindowPosition(const QPoint& /*newPos*/) {
        const QRect& widgetGeometry = _floatingWindow->frameGeometry();
        const QRect& mdiAreaRect = _mdiArea->contentsRect();
        const QRect mdiAreaGeometry(_mdiArea->mapToGlobal(mdiAreaRect.topLeft()),
                                    _mdiArea->mapToGlobal(mdiAreaRect.bottomRight()));
        const QRect& intersection = widgetGeometry & mdiAreaGeometry;

        // Dock the floating window's widget if at least 60% of it is over the MDI area
        if (widgetGeometry.width() * widgetGeometry.height() * 3 <
                intersection.width() * intersection.height() * 5) {
            _floatingWindow->stopWindowDrag();
            _floatingWindow->hide();

            QWidget* widget = _floatingWindow->widget();
            _dockedWindow = this->newDockedWindow(widget);
            _dockedWindow->setWindowTitle(this->windowTitle());

            _floatingWindow->deleteLater();
            _floatingWindow = 0;
            _docked = true;
            _mdiArea->addSubWindow(_dockedWindow);

            // Dragging the window doesn't make sense in tabbed mode
            if (_mdiArea->viewMode() == QMdiArea::SubWindowView)
                _dockedWindow->forceWindowDrag();
        }
    }

    void MdiDockableWindow::trackDockedWindowPosition(const QPoint& /*newPos*/) {
        const QRect& subWindowGeometry = _dockedWindow->frameGeometry();
        const QRect& mdiAreaGeometry = _mdiArea->contentsRect();
        const QRect& intersection = subWindowGeometry & mdiAreaGeometry;

        // Detach the docked window's widget if at least 60% of it has left the MDI area
        if (subWindowGeometry.width() * subWindowGeometry.height() * 2 >
                intersection.width() * intersection.height() * 5) {
            QWidget* widget = _dockedWindow->widget();
            _dockedWindow->stopWindowDrag();
            _dockedWindow->setWidget(0);
            _mdiArea->removeSubWindow(_dockedWindow);

            _floatingWindow = new MdiFloatingWindow(widget, this);
            _floatingWindow->setWindowTitle(this->windowTitle());

            _dockedWindow->deleteLater();
            _dockedWindow = 0;
            _docked = false;

            _floatingWindow->show();
            _floatingWindow->activateWindow();
            _floatingWindow->forceWindowDrag();

            /*
             * Connect signals last so that the floating window's initial move events are ignored.
             * They mustn't be handled because they may contain outdated position information which
             * could, in extreme cases, trigger immediate re-docking of the floating window,
             * leading to all sorts of problems.
             */
            this->connect(_floatingWindow, SIGNAL(s_closed()), SLOT(handleWindowClosing()));
            this->connect(_floatingWindow, SIGNAL(s_positionChanged(const QPoint&)),
                          SLOT(trackFloatingWindowPosition(const QPoint&)));
        }
    }

}
