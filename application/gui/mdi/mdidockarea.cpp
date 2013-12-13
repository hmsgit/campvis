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

#include "mdidockarea.h"

#include <QMenu>

namespace campvis {

    MdiDockArea::MdiDockArea(QWidget* parent /*= 0*/)
        : QMdiArea(parent)
        , _menu(0)
    {
        this->setTabsClosable(true);
        this->setTabsMovable(true);
        this->setDocumentMode(true);

        // Menu setup
        _menu = new QMenu(this);
        QActionGroup* displayStyleActions = new QActionGroup(this);

        QAction* displayTiledAction = displayStyleActions->addAction(tr("Display tiled"));
        displayTiledAction->setCheckable(true);
        displayTiledAction->setChecked(true);
        _menu->addAction(displayTiledAction);
        this->connect(displayTiledAction, SIGNAL(triggered()), SLOT(switchToTiledDisplay()));

        QAction* displayTabbedAction = displayStyleActions->addAction(tr("Display tabbed"));
        displayTabbedAction->setCheckable(true);
        _menu->addAction(displayTabbedAction);
        this->connect(displayTabbedAction, SIGNAL(triggered()), SLOT(switchToTabbedDisplay()));

        _menu->addSeparator();
    }

    MdiDockableWindow* MdiDockArea::addWidget(QWidget* widget, Qt::WindowFlags windowFlags /*= 0*/) {
        MdiDockableWindow* dockableWindow = new MdiDockableWindow(widget, this, windowFlags);
        _menu->addAction(dockableWindow->toggleViewAction());

        return dockableWindow;
    }

    QMdiSubWindow* MdiDockArea::addSubWindow(QMdiSubWindow* mdiSubWindow) {
        QMdiArea::addSubWindow(mdiSubWindow);
        mdiSubWindow->show();

        // Calling tileSubWindows() in TabbedView mode breaks the tabbed display
        if (this->viewMode() == QMdiArea::SubWindowView)
            this->tileSubWindows();

        return mdiSubWindow;
    }

    void MdiDockArea::removeSubWindow(QMdiSubWindow* mdiSubWindow) {
        if (this->activeSubWindow() == mdiSubWindow)
            this->activateNextSubWindow();

        QMdiArea::removeSubWindow(mdiSubWindow);

        // Calling tileSubWindows() in TabbedView mode breaks the tabbed display
        if (this->viewMode() == QMdiArea::SubWindowView)
            this->tileSubWindows();
    }

    QMenu* MdiDockArea::menu() const {
        return _menu;
    }

    void MdiDockArea::switchToTiledDisplay() {
        this->setViewMode(QMdiArea::SubWindowView);
        this->tileSubWindows();
    }

    void MdiDockArea::switchToTabbedDisplay() {
        this->setViewMode(QMdiArea::TabbedView);
    }

}
