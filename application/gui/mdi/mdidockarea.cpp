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
