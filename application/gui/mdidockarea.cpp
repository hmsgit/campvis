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

    PipelineMdiSubWindow* MdiDockArea::addSubWindow(QWidget* widget, Qt::WindowFlags windowFlags /*= 0*/) {
        PipelineMdiSubWindow* mdiSubWindow = new PipelineMdiSubWindow;

        mdiSubWindow->setWidget(widget);
        QMdiArea::addSubWindow(mdiSubWindow, windowFlags);
        widget->show();
        this->tileSubWindows();

        connect(mdiSubWindow, SIGNAL(s_positionChanged(PipelineMdiSubWindow*, const QPoint&)),
                this, SLOT(trackMdiSubWindowsPosition(PipelineMdiSubWindow*, const QPoint&)));

        return mdiSubWindow;
    }

    void MdiDockArea::trackFloatingWindowsPosition(VisualizationPipelineWidget* pipelineWidget, const QPoint& newPos) {
        const QRect& widgetGeometry = pipelineWidget->frameGeometry();
        const QRect& mdiAreaRect = this->contentsRect();
        const QRect mdiAreaGeometry(this->mapToGlobal(mdiAreaRect.topLeft()),
                                    this->mapToGlobal(mdiAreaRect.bottomRight()));
        const QRect& intersection = widgetGeometry & mdiAreaGeometry;

        // Dock the widget if at least 60% of it is over the MDI area
        if (widgetGeometry.width() * widgetGeometry.height() * 3 <
                intersection.width() * intersection.height() * 5) {
            pipelineWidget->stopWindowDrag();

            QWidget* widget = pipelineWidget->canvas();
            PipelineMdiSubWindow* mdiSubWindow = this->addSubWindow(widget);
            mdiSubWindow->setWindowTitle(pipelineWidget->windowTitle());
            widget->show();
            pipelineWidget->deleteLater();

            mdiSubWindow->move(this->mapFromGlobal(newPos));
            mdiSubWindow->grabMouse();
        }
    }

    void MdiDockArea::trackMdiSubWindowsPosition(PipelineMdiSubWindow *mdiSubWindow, const QPoint& newPos) {
        const QRect& subWindowGeometry = mdiSubWindow->frameGeometry();
        const QRect& mdiAreaGeometry = contentsRect();
        const QRect& intersection = subWindowGeometry & mdiAreaGeometry;

        // Detach the subwindow if at least 60% of it has left the MDI area
        if (subWindowGeometry.width() * subWindowGeometry.height() * 2 >
                intersection.width() * intersection.height() * 5) {
            QWidget* widget = mdiSubWindow->widget();
            mdiSubWindow->stopWindowDrag();
            mdiSubWindow->setWidget(0);
            removeSubWindow(mdiSubWindow);
            mdiSubWindow->deleteLater();
            tileSubWindows();

            VisualizationPipelineWidget* pipelineWidget = new VisualizationPipelineWidget(widget);
            pipelineWidget->setWindowTitle(mdiSubWindow->windowTitle());
            pipelineWidget->move(this->mapToGlobal(newPos));
            pipelineWidget->show();
            pipelineWidget->forceWindowDrag();

            connect(pipelineWidget, SIGNAL(s_positionChanged(VisualizationPipelineWidget*, const QPoint&)),
                    this, SLOT(trackFloatingWindowsPosition(VisualizationPipelineWidget*, const QPoint&)));
        }
    }

}
