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

namespace campvis {

    MdiDockedWindow* MdiDockArea::addSubWindow(QWidget* widget, Qt::WindowFlags windowFlags /*= 0*/) {
        MdiDockedWindow* dockedWindow = new MdiDockedWindow();

        dockedWindow->setWidget(widget);
        QMdiArea::addSubWindow(dockedWindow, windowFlags);
        widget->show();
        this->tileSubWindows();

        connect(dockedWindow, SIGNAL(s_positionChanged(MdiDockedWindow*, const QPoint&)),
                this, SLOT(trackMdiSubWindowsPosition(MdiDockedWindow*, const QPoint&)));

        return dockedWindow;
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

            QWidget* widget = floatingWindow->widget();
            MdiDockedWindow* dockedWindow = this->addSubWindow(widget);
            dockedWindow->setWindowTitle(floatingWindow->windowTitle());
            widget->show();
            floatingWindow->deleteLater();

            dockedWindow->move(this->mapFromGlobal(newPos));
            dockedWindow->grabMouse();
        }
    }

    void MdiDockArea::trackMdiSubWindowsPosition(MdiDockedWindow *dockedWindow, const QPoint& newPos) {
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
            dockedWindow->deleteLater();
            tileSubWindows();

            MdiFloatingWindow* floatingWindow = new MdiFloatingWindow(widget);
            floatingWindow->setWindowTitle(dockedWindow->windowTitle());
            floatingWindow->move(this->mapToGlobal(newPos));
            floatingWindow->show();
            floatingWindow->forceWindowDrag();

            connect(floatingWindow, SIGNAL(s_positionChanged(MdiFloatingWindow*, const QPoint&)),
                    this, SLOT(trackFloatingWindowsPosition(MdiFloatingWindow*, const QPoint&)));
        }
    }

}
