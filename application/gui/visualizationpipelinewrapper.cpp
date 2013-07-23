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

#include "visualizationpipelinewrapper.h"

namespace campvis {

    VisualizationPipelineWrapper::VisualizationPipelineWrapper(const std::string& name, QWidget* canvas,
                                                               QMdiArea* mdiArea, QObject* parent /*= 0*/)
        : QObject(parent)
        , _mdiArea(mdiArea)
    {
        _pipelineWidget = new VisualizationPipelineWidget(canvas);
        _mdiSubWindow = new PipelineMdiSubWindow();

        _mdiSubWindow->setWidget(_pipelineWidget);
        mdiArea->addSubWindow(_mdiSubWindow);

        const QString& windowTitle = QString::fromStdString(name);
        _mdiSubWindow->setWindowTitle(windowTitle);
        _pipelineWidget->setWindowTitle(windowTitle);

        connect(_mdiSubWindow, SIGNAL(s_positionChanged(const QPoint&)),
                this, SLOT(trackMdiSubWindowsPosition(const QPoint&)));

        connect(_pipelineWidget, SIGNAL(s_positionChanged(const QPoint&)),
                this, SLOT(trackFloatingWindowsPosition(const QPoint&)));
    }

    void VisualizationPipelineWrapper::trackFloatingWindowsPosition(const QPoint& newPos) {
        const QRect& widgetGeometry = _pipelineWidget->frameGeometry();
        const QRect& mdiAreaRect = _mdiArea->contentsRect();
        const QRect mdiAreaGeometry(_mdiArea->mapToGlobal(mdiAreaRect.topLeft()),
                                    _mdiArea->mapToGlobal(mdiAreaRect.bottomRight()));
        const QRect& intersection = widgetGeometry & mdiAreaGeometry;

        // Dock the widget if at least 60% of it is over the MDI area
        if (widgetGeometry.width() * widgetGeometry.height() * 3 <
                intersection.width() * intersection.height() * 5) {
            _pipelineWidget->stopWindowDrag();

            _mdiSubWindow->setWidget(_pipelineWidget);
            _mdiArea->addSubWindow(_mdiSubWindow);
            _pipelineWidget->show();

            _mdiSubWindow->move(_mdiArea->mapFromGlobal(newPos));
            _mdiSubWindow->grabMouse();
        }
    }

    void VisualizationPipelineWrapper::trackMdiSubWindowsPosition(const QPoint& newPos) {
        const QRect& subWindowGeometry = _mdiSubWindow->frameGeometry();
        const QRect& mdiAreaGeometry = _mdiArea->contentsRect();
        const QRect& intersection = subWindowGeometry & mdiAreaGeometry;

        // Detach the subwindow if at least 60% of it has left the MDI area
        if (subWindowGeometry.width() * subWindowGeometry.height() * 2 >
                intersection.width() * intersection.height() * 5) {
            _mdiSubWindow->stopWindowDrag();
            _mdiSubWindow->setWidget(0);
            _mdiArea->removeSubWindow(_mdiSubWindow);
            _mdiArea->tileSubWindows();

            _pipelineWidget->move(_mdiArea->mapToGlobal(newPos));
            _pipelineWidget->show();
            _pipelineWidget->forceWindowDrag();
        }
    }

}
