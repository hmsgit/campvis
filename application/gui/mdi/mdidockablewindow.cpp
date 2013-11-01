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
        _dockedWindow = this->newDockedWindow(widget);
        _dockedWindow->setWindowFlags(windowFlags);

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

    QAction* MdiDockableWindow::toggleViewAction() const {
        return _toggleViewAction;
    }

    void MdiDockableWindow::setVisible(bool visible) {
        _toggleViewAction->setChecked(visible);
    }

    MdiDockedWindow* MdiDockableWindow::newDockedWindow(QWidget* widget) {
        MdiDockedWindow* dockedWindow = new MdiDockedWindow(_mdiArea);
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

            _floatingWindow = new MdiFloatingWindow(widget);
            _floatingWindow->setWindowTitle(this->windowTitle());
            _floatingWindow->forceWindowDrag();
            this->connect(_floatingWindow, SIGNAL(s_positionChanged(const QPoint&)),
                          SLOT(trackFloatingWindowPosition(const QPoint&)));
            this->connect(_floatingWindow, SIGNAL(s_closed()), SLOT(handleWindowClosing()));

            _dockedWindow->deleteLater();
            _dockedWindow = 0;
            _docked = false;
            _floatingWindow->show();
        }
    }

}
