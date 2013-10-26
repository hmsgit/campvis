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
#include "tgt/assert.h"

#include <QMenu>
#include <QSignalMapper>

namespace campvis {

    MdiDockArea::MdiDockArea(QWidget* parent /*= 0*/)
        : QMdiArea(parent)
        , _menu(0)
        , _signalMapper(0)
    {
        _signalMapper = new QSignalMapper(this);
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

    MdiDockedWindow* MdiDockArea::addSubWindow(QWidget* widget, Qt::WindowFlags windowFlags /*= 0*/) {
        MdiDockedWindow* dockedWindow = new MdiDockedWindow(this, windowFlags);

        dockedWindow->setWidget(widget);
        this->addDockedWindow(dockedWindow);

        // Calling tileSubWindows() in TabbedView mode breaks the tabbed display
        if (this->viewMode() == QMdiArea::SubWindowView)
            this->tileSubWindows();

        QAction* visibilityAction = _menu->addAction(widget->windowTitle());
        visibilityAction->setCheckable(true);
        visibilityAction->setChecked(true);
        visibilityAction->setData(QVariant::fromValue(dynamic_cast<QWidget*>(widget)));

        connect(visibilityAction, SIGNAL(triggered()), _signalMapper, SLOT(map()));
        _signalMapper->setMapping(visibilityAction, visibilityAction);
        connect(_signalMapper, SIGNAL(mapped(QObject*)), SLOT(toggleSubWindowVisibility(QObject*)));

        return dockedWindow;
    }

    QMenu* MdiDockArea::menu() {
        return _menu;
    }

    void MdiDockArea::addDockedWindow(MdiDockedWindow* dockedWindow) {
        QMdiArea::addSubWindow(dockedWindow);
        dockedWindow->show();

        connect(dockedWindow, SIGNAL(s_positionChanged(MdiDockedWindow*, const QPoint&)),
                SLOT(trackMdiSubWindowsPosition(MdiDockedWindow*, const QPoint&)));
    }

    void MdiDockArea::toggleSubWindowVisibility(QObject* actionObject) {
        /*
         * This static_cast is safe as toggleSubWindowVisibility() is a private slot to which only
         * MdiDockArea creates connections, always passing a QAction instance.
         */
        QAction* visibilityAction = static_cast<QAction*>(actionObject);
        QWidget* windowWidget = visibilityAction->data().value<QWidget*>()->parentWidget();

        if (MdiDockedWindow* dockedWindow = dynamic_cast<MdiDockedWindow*>(windowWidget)) {
            if (visibilityAction->isChecked())
                this->addDockedWindow(dockedWindow);
            else
                this->removeSubWindow(dockedWindow);

            // Calling tileSubWindows() in TabbedView mode breaks the tabbed display
            if (this->viewMode() == QMdiArea::SubWindowView)
                this->tileSubWindows();
        } else if (MdiFloatingWindow* floatingWindow = dynamic_cast<MdiFloatingWindow*>(windowWidget)) {
            if (visibilityAction->isChecked())
                floatingWindow->show();
            else
                floatingWindow->hide();
        }
        else
            tgtAssert(false, "Widget's parent is of unsupported type.");
    }

    void MdiDockArea::switchToTiledDisplay() {
        this->setViewMode(QMdiArea::SubWindowView);
        this->tileSubWindows();
    }

    void MdiDockArea::switchToTabbedDisplay() {
        this->setViewMode(QMdiArea::TabbedView);
    }

    void MdiDockArea::trackFloatingWindowsPosition(MdiFloatingWindow* floatingWindow, const QPoint& newPos) {
        const QRect& widgetGeometry = floatingWindow->frameGeometry();
        const QRect& mdiAreaRect = this->contentsRect();
        const QRect mdiAreaGeometry(this->mapToGlobal(mdiAreaRect.topLeft()),
                                    this->mapToGlobal(mdiAreaRect.bottomRight()));
        const QRect& intersection = widgetGeometry & mdiAreaGeometry;

        // Dock the floating window's widget if at least 60% of it is over the MDI area
        if (widgetGeometry.width() * widgetGeometry.height() * 3 <
                intersection.width() * intersection.height() * 5) {
            floatingWindow->stopWindowDrag();
            floatingWindow->hide();

            QWidget* widget = floatingWindow->widget();
            MdiDockedWindow* dockedWindow = new MdiDockedWindow();
            dockedWindow->setWidget(widget);
            dockedWindow->setWindowTitle(floatingWindow->windowTitle());

            this->addDockedWindow(dockedWindow);
            widget->show();
            floatingWindow->deleteLater();
            dockedWindow->forceWindowDrag();
        }
    }

    void MdiDockArea::trackMdiSubWindowsPosition(MdiDockedWindow *dockedWindow, const QPoint& /*newPos*/) {
        const QRect& subWindowGeometry = dockedWindow->frameGeometry();
        const QRect& mdiAreaGeometry = contentsRect();
        const QRect& intersection = subWindowGeometry & mdiAreaGeometry;

        // Detach the docked window's widget if at least 60% of it has left the MDI area
        if (subWindowGeometry.width() * subWindowGeometry.height() * 2 >
                intersection.width() * intersection.height() * 5) {
            QWidget* widget = dockedWindow->widget();
            dockedWindow->stopWindowDrag();
            dockedWindow->setWidget(0);
            removeSubWindow(dockedWindow);

            MdiFloatingWindow* floatingWindow = new MdiFloatingWindow(widget);
            floatingWindow->setWindowTitle(dockedWindow->windowTitle());
            floatingWindow->forceWindowDrag();
            connect(floatingWindow, SIGNAL(s_positionChanged(MdiFloatingWindow*, const QPoint&)),
                    this, SLOT(trackFloatingWindowsPosition(MdiFloatingWindow*, const QPoint&)));

            dockedWindow->deleteLater();
            tileSubWindows();
            floatingWindow->show();
        }
    }

}
